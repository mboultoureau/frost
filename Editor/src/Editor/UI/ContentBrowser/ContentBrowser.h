#pragma once

#include "Editor/Project/ProjectInfo.h"

#include <filesystem>
#include <vector>
#include <memory>
#include <string>

namespace Editor
{
    class FileWatcher;
    class AssetIconManager;

    struct FileEntry
    {
        std::filesystem::path Path;
        std::string Name;
        std::string Extension;
        bool IsDirectory;
    };

    class ContentBrowser
    {
    public:
        ContentBrowser(const ProjectInfo& projectInfo);
        ~ContentBrowser();

        void Draw();

    private:
        void _RefreshAssetList();
        void _OnFileSystemChanged();

        // Rendering
        void _RenderBreadCrumbs();
        void _RenderTree();
        void _RenderTreeRecursive(const std::filesystem::path& directory);
        void _RenderArea();
        void _RenderModals();

        // Actions
        void _CreatePrefab(const std::string& name);
        void _CreateFolder(const std::string& name);
        void _RenameItem(const std::filesystem::path& oldPath, const std::string& newName);
        void _DeleteItem(const std::filesystem::path& path);
        void _MoveAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destDir);

        void _ShowInExplorer(const std::filesystem::path& path);
        void _OpenFileInExternalEditor(const std::filesystem::path& path);

    private:
        const ProjectInfo& _projectInfo;

        // Modules
        std::unique_ptr<FileWatcher> _fileWatcher;
        std::unique_ptr<AssetIconManager> _iconManager;

        // Navigation state
        std::filesystem::path _assetDirectory;
        std::filesystem::path _sourceDirectory;
        std::filesystem::path _currentDirectory;
        std::vector<FileEntry> _currentDirCache;

        // Settings
        float _thumbnailSize = 96.0f;
        float _padding = 16.0f;
        std::vector<std::string> _ignoredExtensions = { ".meta", ".bin" };
        bool _dirty = true;

        // Context Menu and modals
        std::filesystem::path _contextMenuPath;

        bool _showCreatePrefabPopup = false;
        char _newPrefabNameBuffer[128] = "";

        bool _showRenameModal = false;
        char _renameBuffer[256] = "";

        bool _showDeleteModal = false;

        bool _showCreateFolderModal = false;
        char _createFolderBuffer[256] = "NewFolder";
    };
} // namespace Editor