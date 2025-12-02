#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"

#include <map>
#include <memory>
#include <future>
#include <deque>
#include <mutex>
#include <functional>

namespace Frost
{
    class AssetManager
    {
    public:
        static void Shutdown();
        static void Update();
        static void PruneUnused();

        // Generic asset loading function
        template<typename T, typename... Args>
        static std::shared_ptr<T> LoadAsset(const Asset::Path& path, Args&&... args)
            requires(!std::is_same_v<T, Texture>)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must be derived from Frost::Asset");

            // Check if asset is already loaded
            {
                std::unique_lock lock(_mutex);
                auto it = _loadedAssets.find(path);
                if (it != _loadedAssets.end())
                {
                    return std::static_pointer_cast<T>(it->second);
                }
            }

            std::shared_ptr<T> asset = std::make_shared<T>();

            {
                std::unique_lock lock(_mutex);
                _loadedAssets[path] = asset;
            }

            // Load new asset
            std::thread(
                [asset, path, args...]() mutable
                {
                    try
                    {
                        // Load asset data on CPU
                        asset->SetStatus(AssetStatus::Loading);
                        asset->LoadCPU(path, std::forward<Args>(args)...);

                        // Queue GPU upload
                        std::lock_guard queueLock(_queueMutex);
                        _uploadQueue.emplace_back([asset]() { asset->UploadGPU(); });
                    }
                    catch (const std::exception& e)
                    {
                        asset->SetStatus(AssetStatus::Failed);
                        FT_ENGINE_ERROR("Async load exception '{}': {}", path, e.what());
                        return;
                    }
                })
                .detach();

            return asset;
        }

        // Specialized function to get already loaded asset
        static std::shared_ptr<Texture> LoadAsset(const Asset::Path& path, TextureConfig& config);

    private:
        static std::mutex _mutex;
        static std::map<Asset::Path, std::shared_ptr<Asset>> _loadedAssets;

        static std::mutex _queueMutex;
        static std::deque<std::function<void()>> _uploadQueue;

        // 5ms budget per frame
        static constexpr int _timeBudget = 5;
    };
} // namespace Frost
