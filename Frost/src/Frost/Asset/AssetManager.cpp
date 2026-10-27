#include "Frost/Asset/AssetManager.h"
#include "Frost/Core/Application.h"
#include <assimp/texture.h>

namespace Frost
{
    namespace
    {
        uint32_t s_frameCounter = 0;
    } // namespace

    FROST_API std::map<Asset::Path, std::shared_ptr<Asset>> AssetManager::_loadedAssets;
    FROST_API std::mutex AssetManager::_mutex;
    FROST_API std::deque<std::function<void()>> AssetManager::_uploadQueue;
    FROST_API std::mutex AssetManager::_queueMutex;
    std::condition_variable AssetManager::_cvUploadQueueNotFull;
    bool AssetManager::_useMultiThreading = true;
    int AssetManager::_timeBudget = 4;
    std::atomic<int> AssetManager::_activeLoadingThreads = 0;
    std::atomic<int> AssetManager::_inFlightJobs = 0;
    std::atomic<bool> AssetManager::_isShuttingDown = false;
    std::condition_variable AssetManager::_cvLoadingDone;
    std::mutex AssetManager::_shutdownMutex;

    void AssetManager::SetUseMultiThreading(bool useMultiThreading)
    {
        _useMultiThreading = useMultiThreading;
    }

    bool AssetManager::GetUseMultiThreading()
    {
        return _useMultiThreading;
    }

    void AssetManager::SetTimeBudget(int ms)
    {
        _timeBudget = std::max(1, ms);
    }

    int AssetManager::GetTimeBudget()
    {
        return _timeBudget;
    }

    int AssetManager::GetActiveLoadingThreads()
    {
        return _activeLoadingThreads.load();
    }

    bool AssetManager::IsShuttingDown()
    {
        return _isShuttingDown.load(std::memory_order_acquire);
    }

    Asset::Path AssetManager::NormalizePath(const Asset::Path& path)
    {
        if (path.empty())
        {
            return path;
        }

        auto queryPos = path.find('?');
        if (queryPos != std::string::npos)
        {
            std::string base = path.substr(0, queryPos);
            std::string query = path.substr(queryPos);
            return std::filesystem::path(base).lexically_normal().generic_string() + query;
        }

        return std::filesystem::path(path).lexically_normal().generic_string();
    }

    void AssetManager::ThrottleCPUWorkers()
    {
        constexpr size_t maxPendingUploads = 8;
        std::unique_lock<std::mutex> lock(_queueMutex);
        _cvUploadQueueNotFull.wait_for(
            lock,
            std::chrono::milliseconds(10),
            []()
            { return _isShuttingDown.load(std::memory_order_acquire) || _uploadQueue.size() < maxPendingUploads; });
    }

    void AssetManager::EnqueueUpload(const Asset::Path& path, std::weak_ptr<Asset> weakAsset)
    {
        if (_isShuttingDown.load(std::memory_order_acquire))
        {
            return;
        }

        AddToUploadQueue(
            [path, weakAsset]()
            {
                auto asset = weakAsset.lock();
                if (!asset)
                {
                    return;
                }

                if (asset->IsCancelled())
                {
                    RemoveRegisteredAsset(path, *asset);
                    return;
                }

                AsyncJobGuard jobGuard(*asset);

                if (asset->IsCancelled() || asset.use_count() <= 1 + asset->GetInternalRefCount())
                {
                    asset->Cancel();
                    RemoveRegisteredAsset(path, *asset);
                    return;
                }

                if (asset->UploadGPUChunk())
                {
                    EnqueueUpload(path, weakAsset);
                }
                else
                {
                    if (asset.use_count() <= 1 + asset->GetInternalRefCount())
                    {
                        asset->Cancel();
                        RemoveRegisteredAsset(path, *asset);
                    }
                }
            });
    }

    void AssetManager::DispatchLoadJob(std::function<void()> job)
    {
        if (_isShuttingDown.load(std::memory_order_acquire))
        {
            return;
        }

        _inFlightJobs++;

        auto wrappedJob = [job = std::move(job)]()
        {
            _activeLoadingThreads++;
            if (!_isShuttingDown.load(std::memory_order_acquire))
            {
                job();
            }
            _activeLoadingThreads--;

            if (--_inFlightJobs == 0)
            {
                std::scoped_lock lock(_shutdownMutex);
                _cvLoadingDone.notify_all();
            }
        };

        if (_useMultiThreading && Application::HasInstance())
        {
            if (Application::GetJobScheduler().AddJob(wrappedJob))
            {
                return;
            }
        }

        wrappedJob();
    }

    std::shared_ptr<Asset> AssetManager::FindAsset(const Asset::Path& rawPath)
    {
        Asset::Path path = NormalizePath(rawPath);
        std::scoped_lock lock(_mutex);
        auto it = _loadedAssets.find(path);
        if (it != _loadedAssets.end())
        {
            return it->second;
        }
        return nullptr;
    }

    bool AssetManager::HasAsset(const Asset::Path& rawPath)
    {
        auto asset = FindAsset(rawPath);
        return asset && !asset->IsCancelled() && asset->GetStatus() != AssetStatus::Failed;
    }

    std::pair<std::shared_ptr<Asset>, bool> AssetManager::FindOrRegisterPlaceholder(const Asset::Path& rawPath,
                                                                                    std::shared_ptr<Asset> placeholder)
    {
        Asset::Path path = NormalizePath(rawPath);
        std::scoped_lock lock(_mutex);
        auto it = _loadedAssets.find(path);
        if (it != _loadedAssets.end())
        {
            if (it->second->IsCancelled() || it->second->GetStatus() == AssetStatus::Failed)
            {
                _loadedAssets[path] = placeholder;
                return { placeholder, true };
            }
            return { it->second, false };
        }
        _loadedAssets[path] = placeholder;
        return { placeholder, true };
    }

    bool AssetManager::RemoveRegisteredAsset(const Asset::Path& rawPath, const Asset& asset)
    {
        Asset::Path path = NormalizePath(rawPath);
        std::scoped_lock lock(_mutex);
        auto it = _loadedAssets.find(path);
        if (it != _loadedAssets.end() && it->second.get() == &asset)
        {
            _loadedAssets.erase(it);
            return true;
        }
        return false;
    }

    void AssetManager::AddToUploadQueue(std::function<void()>&& job)
    {
        if (_isShuttingDown.load(std::memory_order_acquire))
        {
            return;
        }

        std::scoped_lock queueLock(_queueMutex);
        _uploadQueue.emplace_back(std::move(job));
    }

    void AssetManager::Shutdown()
    {
        _isShuttingDown.store(true, std::memory_order_release);
        _cvUploadQueueNotFull.notify_all();

        {
            std::unique_lock lock(_shutdownMutex);
            _cvLoadingDone.wait(lock, []() { return _inFlightJobs.load(std::memory_order_acquire) == 0; });
        }

        {
            std::scoped_lock lock(_queueMutex);
            _uploadQueue.clear();
        }

        {
            std::scoped_lock lock(_mutex);
            _loadedAssets.clear();
        }

        _isShuttingDown.store(false, std::memory_order_release);
    }

    void AssetManager::Update()
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        while (true)
        {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() >= _timeBudget)
            {
                break;
            }

            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(_queueMutex, std::try_to_lock);
                if (!lock.owns_lock() || _uploadQueue.empty())
                {
                    break;
                }
                job = std::move(_uploadQueue.front());
                _uploadQueue.pop_front();
            }

            _cvUploadQueueNotFull.notify_one();

            if (job)
            {
                job();
            }
        }

        if (++s_frameCounter % 60 == 0)
        {
            PruneUnused();
        }
    }

    void AssetManager::PruneUnused()
    {
        std::scoped_lock lock(_mutex);
        bool prunedAny;
        do
        {
            prunedAny = false;
            for (auto it = _loadedAssets.begin(); it != _loadedAssets.end();)
            {
                if (it->second->GetInternalRefCount() > 0)
                {
                    ++it;
                    continue;
                }

                bool isUnused = (it->second.use_count() == 1) || (it->second->IsCancelled());

                if (isUnused)
                {
                    it = _loadedAssets.erase(it);
                    prunedAny = true;
                }
                else
                {
                    ++it;
                }
            }
        } while (prunedAny);
    }

    bool AssetManager::UnloadAsset(const Asset::Path& rawPath)
    {
        Asset::Path path = NormalizePath(rawPath);
        std::scoped_lock lock(_mutex);
        auto it = _loadedAssets.find(path);
        if (it == _loadedAssets.end())
        {
            return false;
        }

        _loadedAssets.erase(it);
        return true;
    }

    std::shared_ptr<Texture> AssetManager::LoadAsset(const Asset::Path& rawPath, TextureConfig& config)
    {
        if (_isShuttingDown.load(std::memory_order_acquire))
        {
            return nullptr;
        }

        Asset::Path path = NormalizePath(rawPath);
        std::shared_ptr<Texture> texture;

        {
            std::scoped_lock lock(_mutex);
            if (auto it = _loadedAssets.find(path); it != _loadedAssets.end())
            {
                if (!it->second->IsCancelled() && it->second->GetStatus() != AssetStatus::Failed)
                {
                    return std::static_pointer_cast<Texture>(it->second);
                }
            }

            config.loadImmediately = false;
            texture = Texture::Create(config);
            _loadedAssets[path] = texture;
        }

        std::weak_ptr<Texture> weakTexture = texture;
        auto loadJob = [weakTexture, path, config]() mutable
        {
            auto texture = weakTexture.lock();
            if (!texture || texture->IsCancelled())
            {
                return;
            }

            AsyncJobGuard jobGuard(*texture);
            ThrottleCPUWorkers();

            try
            {
                if (texture->IsCancelled() || texture.use_count() <= 1 + texture->GetInternalRefCount())
                {
                    texture->Cancel();
                    RemoveRegisteredAsset(path, *texture);
                    return;
                }

                texture->SetStatus(AssetStatus::Loading);
                texture->LoadCPU(path, config);

                if (texture->IsCancelled() || texture.use_count() <= 1 + texture->GetInternalRefCount())
                {
                    texture->Cancel();
                    RemoveRegisteredAsset(path, *texture);
                    return;
                }

                EnqueueUpload(path, weakTexture);
            }
            catch (const std::exception& e)
            {
                texture->SetStatus(AssetStatus::Failed);
                FT_ENGINE_ERROR("Async load texture exception '{}': {}", path, e.what());
            }
        };

        DispatchLoadJob(std::move(loadJob));

        return texture;
    }
} // namespace Frost
