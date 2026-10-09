#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Renderer/BoundingBox.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Editor
{
    struct Thumbnail
    {
        std::string pathKey;
        std::vector<uint8_t> pixels;
        uint32_t width = 0;
        uint32_t height = 0;
        std::filesystem::file_time_type lastWriteTime;
    };

    class AssetIconManager
    {
    public:
        AssetIconManager();
        ~AssetIconManager();

        void Update();

        std::shared_ptr<Frost::Texture> GetIcon(const std::filesystem::path& path, bool isDirectory);

        void ClearCache();

    private:
        std::shared_ptr<Frost::Texture> _GenerateModelThumbnail(const std::filesystem::path& path);
        void _FocusCameraOnBounds(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds);
        bool _IsModelFormat(const std::string& extension);
        bool _IsImageFormat(const std::string& extension);
        std::filesystem::path _GetCachePath(const std::filesystem::path& assetPath);

        void _WorkerLoop();
        bool _ProcessImageThumbnail(const std::filesystem::path& assetPath, Thumbnail& outResult);

    private:
        struct CacheEntry
        {
            std::shared_ptr<Frost::Texture> texture;
            std::filesystem::file_time_type lastWriteTime;
        };

        std::shared_ptr<Frost::Texture> _folderIcon;
        std::shared_ptr<Frost::Texture> _fileIcon;
        std::unordered_map<std::string, CacheEntry> _iconCache;
        std::deque<std::filesystem::path> _loadQueue;
        std::unordered_set<std::string> _pendingPaths;

        std::filesystem::path _thumbnailCacheDir;

        // Image thumbnail generation
        std::thread _workerThread;
        std::mutex _workMutex;
        std::condition_variable _workCv;
        std::deque<std::filesystem::path> _imageWorkQueue;
        std::vector<Thumbnail> _completedThumbnails;
        std::atomic<bool> _running{ true };
    };
} // namespace Editor