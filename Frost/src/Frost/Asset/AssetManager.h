#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Core/Core.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <filesystem>
#include <utility>

namespace Frost
{
    class FROST_API AssetManager
    {
    public:
        static void Shutdown();
        static void Update();
        static void PruneUnused();
        static bool UnloadAsset(const Asset::Path& path);
        static std::shared_ptr<Asset> FindAsset(const Asset::Path& path);
        static bool HasAsset(const Asset::Path& path);

        static void SetUseMultiThreading(bool useMultiThreading);
        static bool GetUseMultiThreading();

        static void SetTimeBudget(int ms);
        static int GetTimeBudget();

        static int GetActiveLoadingThreads();
        static bool IsShuttingDown();

        static Asset::Path NormalizePath(const Asset::Path& path);
        static std::pair<std::shared_ptr<Asset>, bool> FindOrRegisterPlaceholder(const Asset::Path& path,
                                                                                 std::shared_ptr<Asset> placeholder);

        template<typename T, typename... Args>
        static std::shared_ptr<T> LoadAsset(const Asset::Path& rawPath, Args&&... args)
            requires(!std::is_same_v<T, Texture>)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must be derived from Frost::Asset");

            Asset::Path path = NormalizePath(rawPath);
            auto newAsset = std::make_shared<T>();
            auto [existingOrNew, wasInserted] = FindOrRegisterPlaceholder(path, newAsset);

            if (!wasInserted)
            {
                return std::static_pointer_cast<T>(existingOrNew);
            }

            QueueLoad(path, newAsset, std::forward<Args>(args)...);

            return newAsset;
        }

        static std::shared_ptr<Texture> LoadAsset(const Asset::Path& path, TextureConfig& config);

    private:
        static void AddToUploadQueue(std::function<void()>&& job);
        static void EnqueueUpload(const Asset::Path& path, std::weak_ptr<Asset> asset);
        static void ThrottleCPUWorkers();
        static void DispatchLoadJob(std::function<void()> job);

        static bool RemoveRegisteredAsset(const Asset::Path& path, const Asset& asset);

        template<typename T, typename... Args>
        static void QueueLoad(const Asset::Path& path, std::shared_ptr<T> asset, Args&&... args)
        {
            if (IsShuttingDown())
            {
                asset->SetStatus(AssetStatus::Failed);
                return;
            }

            std::weak_ptr<T> weakAsset = asset;
            auto loadJob = [weakAsset, path, args...]() mutable
            {
                auto asset = weakAsset.lock();
                if (!asset || asset->IsCancelled())
                {
                    return;
                }

                AsyncJobGuard jobGuard(*asset);
                ThrottleCPUWorkers();

                try
                {
                    if (asset->IsCancelled() || asset.use_count() <= 1 + asset->GetInternalRefCount())
                    {
                        asset->Cancel();
                        RemoveRegisteredAsset(path, *asset);
                        return;
                    }

                    asset->SetStatus(AssetStatus::Loading);
                    asset->LoadCPU(path, std::forward<Args>(args)...);

                    if (asset->IsCancelled() || asset.use_count() <= 1 + asset->GetInternalRefCount())
                    {
                        asset->Cancel();
                        RemoveRegisteredAsset(path, *asset);
                        return;
                    }

                    EnqueueUpload(path, weakAsset);
                }
                catch (const std::exception& e)
                {
                    asset->SetStatus(AssetStatus::Failed);
                    FT_ENGINE_ERROR("Async load exception '{}': {}", path, e.what());
                }
            };

            DispatchLoadJob(std::move(loadJob));
        }

    private:
        static std::mutex _mutex;
        static std::map<Asset::Path, std::shared_ptr<Asset>> _loadedAssets;

        static std::mutex _queueMutex;
        static std::deque<std::function<void()>> _uploadQueue;
        static std::condition_variable _cvUploadQueueNotFull;

        static int _timeBudget;

        static bool _useMultiThreading;
        static std::atomic<int> _activeLoadingThreads;
        static std::atomic<int> _inFlightJobs;
        static std::atomic<bool> _isShuttingDown;
        static std::condition_variable _cvLoadingDone;
        static std::mutex _shutdownMutex;
    };
} // namespace Frost
