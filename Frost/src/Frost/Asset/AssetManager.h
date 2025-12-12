#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Core/Core.h"
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
    class FROST_API AssetManager
    {
    public:
        static void Shutdown();
        static void Update();
        static void PruneUnused();

        template<typename T, typename... Args>
        static std::shared_ptr<T> LoadAsset(const Asset::Path& path, Args&&... args)
            requires(!std::is_same_v<T, Texture>)
        {
            static_assert(std::is_base_of<Asset, T>::value, "T must be derived from Frost::Asset");

            std::shared_ptr<Asset> existingAsset = FindAsset(path);
            if (existingAsset)
            {
                return std::static_pointer_cast<T>(existingAsset);
            }

            std::shared_ptr<T> newAsset = std::make_shared<T>();
            QueueLoad(path, newAsset, std::forward<Args>(args)...);

            return newAsset;
        }

        static std::shared_ptr<Texture> LoadAsset(const Asset::Path& path, TextureConfig& config);

    private:
        static std::shared_ptr<Asset> FindAsset(const Asset::Path& path);
        static void RegisterAsset(const Asset::Path& path, std::shared_ptr<Asset> asset);

        static void AddToUploadQueue(std::function<void()>&& job);

        template<typename T, typename... Args>
        static void QueueLoad(const Asset::Path& path, std::shared_ptr<T> asset, Args&&... args)
        {
            RegisterAsset(path, asset);

            std::thread(
                [asset, path, args...]() mutable
                {
                    try
                    {
                        asset->SetStatus(AssetStatus::Loading);
                        asset->LoadCPU(path, std::forward<Args>(args)...);

                        AddToUploadQueue([asset]() { asset->UploadGPU(); });
                    }
                    catch (const std::exception& e)
                    {
                        asset->SetStatus(AssetStatus::Failed);
                        FT_ENGINE_ERROR("Async load exception '{}': {}", path, e.what());
                    }
                })
                .detach();
        }

    private:
        static std::mutex _mutex;
        static std::map<Asset::Path, std::shared_ptr<Asset>> _loadedAssets;

        static std::mutex _queueMutex;
        static std::deque<std::function<void()>> _uploadQueue;

        static constexpr int _timeBudget = 5;
    };
} // namespace Frost
