#include "Frost/Asset/AssetManager.h"
#include <assimp/texture.h>

namespace Frost
{
    std::map<Asset::Path, std::shared_ptr<Asset>> AssetManager::_loadedAssets;
    std::mutex AssetManager::_mutex;
    std::deque<std::function<void()>> AssetManager::_uploadQueue;
    std::mutex AssetManager::_queueMutex;

    void AssetManager::Shutdown()
    {
        std::unique_lock lock(_mutex);
        _loadedAssets.clear();
    }

    void AssetManager::Update()
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        std::unique_lock<std::mutex> lock(_queueMutex, std::try_to_lock);
        if (!lock.owns_lock())
        {
            return;
        }

        while (!_uploadQueue.empty())
        {
            auto job = _uploadQueue.front();
            _uploadQueue.pop_front();

            job();

            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > 5)
            {
                break;
            }
        }
    }

    void AssetManager::PruneUnused()
    {
        std::unique_lock lock(_mutex);
        for (auto it = _loadedAssets.begin(); it != _loadedAssets.end();)
        {
            if (it->second.use_count() == 1)
            {
                it = _loadedAssets.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    std::shared_ptr<Texture> AssetManager::LoadAsset(const Asset::Path& path, TextureConfig& config)
    {
        {
            std::unique_lock lock(_mutex);
            if (auto it = _loadedAssets.find(path); it != _loadedAssets.end())
            {
                return std::static_pointer_cast<Texture>(it->second);
            }
        }

        config.loadImmediately = false;
        auto texture = Texture::Create(config);

        {
            std::unique_lock lock(_mutex);
            _loadedAssets[path] = texture;
        }

        std::thread(
            [texture, path, config]() mutable
            {
                texture->LoadCPU(path, config);
                {
                    std::lock_guard<std::mutex> queueLock(_queueMutex);
                    _uploadQueue.push_back([texture]() { texture->UploadGPU(); });
                }
            })
            .detach();

        auto format = texture->GetFormat();
        FT_INFO("Texture {}", static_cast<int>(format));

        return texture;
    }
} // namespace Frost
