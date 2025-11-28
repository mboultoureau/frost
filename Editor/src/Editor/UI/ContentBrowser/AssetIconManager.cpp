#include "Editor/UI/ContentBrowser/AssetIconManager.h"

namespace Editor
{
    AssetIconManager::AssetIconManager()
    {
        Frost::TextureConfig folderConfig;
        folderConfig.debugName = "FolderIcon";
        folderConfig.path = "./resources/editor/file_extensions/folder.png";

        Frost::TextureConfig fileConfig;
        fileConfig.debugName = "FileIcon";
        fileConfig.path = "./resources/editor/file_extensions/file.png";

        _folderIcon = Frost::Texture::Create(folderConfig);
        _fileIcon = Frost::Texture::Create(fileConfig);
    }

    std::shared_ptr<Frost::Texture> AssetIconManager::GetIcon(const std::filesystem::path& path, bool isDirectory)
    {
        if (isDirectory)
        {
            return _folderIcon;
        }

        std::string pathKey = path.string();
        std::string ext = path.extension().string();

        // If the file is an image, load and cache its thumbnail
        if (ext == ".png" || ext == ".jpg" || ext == ".dds" || ext == ".tga")
        {
            if (_iconCache.contains(pathKey))
            {
                return _iconCache.at(pathKey);
            }

            Frost::TextureConfig config;
            config.path = path.string();
            config.debugName = pathKey;

            std::shared_ptr<Frost::Texture> tex = Frost::Texture::Create(config);

            if (tex)
            {
                _iconCache[pathKey] = tex;
                return tex;
            }
        }

        return _fileIcon;
    }

    void AssetIconManager::ClearCache()
    {
        _iconCache.clear();
    }
} // namespace Editor