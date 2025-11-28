#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Renderer/BoundingBox.h"

#include <filesystem>
#include <unordered_map>

namespace Editor
{

    class AssetIconManager
    {
    public:
        AssetIconManager();
        ~AssetIconManager() = default;

        std::shared_ptr<Frost::Texture> GetIcon(const std::filesystem::path& path, bool isDirectory);

        void ClearCache();

    private:
        struct CacheEntry
        {
            std::shared_ptr<Frost::Texture> texture;
            std::filesystem::file_time_type lastWriteTime;
        };

        std::shared_ptr<Frost::Texture> _GenerateModelThumbnail(const std::filesystem::path& path);
        void _FocusCameraOnBounds(Frost::Component::Transform& cameraTransform, const Frost::BoundingBox& bounds);
        bool _IsModelFormat(const std::string& extension);

        std::shared_ptr<Frost::Texture> _folderIcon;
        std::shared_ptr<Frost::Texture> _fileIcon;
        std::unordered_map<std::string, CacheEntry> _iconCache;
    };
} // namespace Editor