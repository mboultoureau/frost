#pragma once

#include "Frost/Asset/Texture.h"

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
        std::shared_ptr<Frost::Texture> _folderIcon;
        std::shared_ptr<Frost::Texture> _fileIcon;
        std::unordered_map<std::string, std::shared_ptr<Frost::Texture>> _iconCache;
    };
} // namespace Editor