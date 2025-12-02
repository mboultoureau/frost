#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Renderer/BoundingBox.h"

#include <deque>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace Editor
{

    class AssetIconManager
    {
    public:
        AssetIconManager();
        ~AssetIconManager() = default;

        void Update();

        std::shared_ptr<Frost::Texture> GetIcon(const std::filesystem::path& path, bool isDirectory);

        void ClearCache();

    private:
        std::shared_ptr<Frost::Texture> _GenerateModelThumbnail(const std::filesystem::path& path);
        void _FocusCameraOnBounds(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds);
        bool _IsModelFormat(const std::string& extension);
        std::filesystem::path _GetCachePath(const std::filesystem::path& assetPath);

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
    };
} // namespace Editor