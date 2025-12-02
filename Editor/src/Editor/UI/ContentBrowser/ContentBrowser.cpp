#include "Editor/UI/ContentBrowser/ContentBrowser.h"
#include "Editor/Utils/FileWatcher.h"
#include "Editor/UI/ContentBrowser/AssetIconManager.h"
#include "Editor/UI/ContentBrowser/AssetMetadata.h"
#include "Editor/EditorApp.h"

#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Scene/Components/Meta.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Asset/MeshConfig.h"

#include <imgui.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <shellapi.h>
#include <imgui_internal.h>

using namespace Frost;
using namespace Frost::Component;

namespace Editor
{
    ContentBrowser::ContentBrowser(const ProjectInfo& projectInfo) : _projectInfo(projectInfo)
    {
        _assetDirectory = _projectInfo.GetProjectDir();
        if (!_projectInfo.GetConfig().assetDirectory.empty())
        {
            _assetDirectory /= _projectInfo.GetConfig().assetDirectory;
        }

        _sourceDirectory = _projectInfo.GetProjectDir();
        if (!_projectInfo.GetConfig().sourceDirectory.empty())
        {
            _sourceDirectory /= _projectInfo.GetConfig().sourceDirectory;
        }

        _currentDirectory = _assetDirectory;

        _iconManager = std::make_unique<AssetIconManager>();

        _fileWatcher = std::make_unique<FileWatcher>(_assetDirectory);
        _fileWatcher->SetCallback([this]() { this->_OnFileSystemChanged(); });
        _fileWatcher->Start();

        _RefreshAssetList();
    }

    ContentBrowser::~ContentBrowser()
    {
        if (_fileWatcher)
        {
            _fileWatcher->Stop();
        }
    }

    void ContentBrowser::_OnFileSystemChanged()
    {
        _dirty = true;
    }

    void ContentBrowser::_RefreshAssetList()
    {
        _currentDirCache.clear();

        if (!std::filesystem::exists(_currentDirectory))
        {
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(_currentDirectory))
        {
            auto u8ext = entry.path().extension().u8string();
            std::string ext(u8ext.begin(), u8ext.end());

            if (std::find(_ignoredExtensions.begin(), _ignoredExtensions.end(), ext) != _ignoredExtensions.end())
            {
                continue;
            }

            FileEntry file;
            auto u8path = entry.path().filename().u8string();
            file.Name = std::string(u8path.begin(), u8path.end());

            auto u8fullPath = entry.path().u8string();
            file.Path = std::string(u8fullPath.begin(), u8fullPath.end());

            file.Extension = ext;
            file.IsDirectory = entry.is_directory();

            if (!file.IsDirectory)
            {
                MetadataManager::EnsureMetadata(file.Path);
            }

            _currentDirCache.push_back(file);
        }
        _dirty = false;
    }

    // Rendering
    void ContentBrowser::Draw()
    {
        if (_iconManager)
        {
            _iconManager->Update();
        }

        if (_dirty)
        {
            _RefreshAssetList();
        }

        ImGui::Begin("Content Browser");

        _RenderBreadCrumbs();

        if (ImGui::BeginTable("ContentBrowserLayout", 2, ImGuiTableFlags_Resizable))
        {
            ImGui::TableSetupColumn("Tree", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Grid", ImGuiTableColumnFlags_None);

            ImGui::TableNextRow();

            // Tree View
            ImGui::TableSetColumnIndex(0);
            _RenderTree();

            // Grid View
            ImGui::TableSetColumnIndex(1);
            _RenderArea();

            ImGui::EndTable();
        }

        _RenderModals();

        ImGui::End();
    }

    void ContentBrowser::_RenderBreadCrumbs()
    {
        const std::filesystem::path& baseDir =
            (_currentDirectory.string().find(_sourceDirectory.string()) == 0 || _currentDirectory == _sourceDirectory)
                ? _sourceDirectory
                : _assetDirectory;

        if (_currentDirectory != baseDir)
        {
            if (ImGui::Button("<- Back"))
            {
                std::filesystem::path parent = _currentDirectory.parent_path();
                if (parent.string().find(baseDir.string()) == 0 || parent == baseDir)
                {
                    _currentDirectory = parent;
                    _RefreshAssetList();
                }
            }
            ImGui::SameLine();
        }

        std::string relativePath = std::filesystem::relative(_currentDirectory, baseDir).string();
        if (relativePath.empty() || relativePath == ".")
        {
            relativePath = baseDir.filename().string();
        }

        ImGui::Text("Current: %s", relativePath.c_str());
        ImGui::Separator();
    }

    void ContentBrowser::_RenderTree()
    {
        ImGui::PushID("ProjectTreeRoot");

        // Actions
        auto HandleTreeContext = [&](const std::filesystem::path& path)
        {
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Open"))
                {
                    _currentDirectory = path;
                    _RefreshAssetList();
                }
                if (ImGui::MenuItem("Show in Explorer"))
                    _ShowInExplorer(path);
                ImGui::Separator();
                if (ImGui::MenuItem("Rename"))
                {
                    _contextMenuPath = path;
                    strncpy_s(_renameBuffer, path.filename().string().c_str(), sizeof(_renameBuffer));
                    _showRenameModal = true;
                }
                if (ImGui::MenuItem("Delete"))
                {
                    _contextMenuPath = path;
                    _showDeleteModal = true;
                }
                ImGui::EndPopup();
            }
        };

        // Assets folder
        ImGuiTreeNodeFlags assetsFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen |
                                         ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DrawLinesFull;
        if (_currentDirectory == _assetDirectory || _currentDirectory.string().find(_assetDirectory.string()) == 0)
            assetsFlags |= ImGuiTreeNodeFlags_Selected;

        bool assetsOpened = ImGui::TreeNodeEx("Assets", assetsFlags);
        HandleTreeContext(_assetDirectory);
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        {
            _currentDirectory = _assetDirectory;
            _dirty = true;
        }

        if (assetsOpened)
        {
            _RenderTreeRecursive(_assetDirectory);
            ImGui::TreePop();
        }

        // Source folder
        if (!_projectInfo.GetConfig().sourceDirectory.empty())
        {
            ImGuiTreeNodeFlags sourceFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen |
                                             ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DrawLinesFull;
            if (_currentDirectory == _sourceDirectory ||
                _currentDirectory.string().find(_sourceDirectory.string()) == 0)
            {
                sourceFlags |= ImGuiTreeNodeFlags_Selected;
            }

            bool sourcesOpened = ImGui::TreeNodeEx("Source", sourceFlags);
            HandleTreeContext(_sourceDirectory);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                _currentDirectory = _sourceDirectory;
                _dirty = true;
            }

            if (sourcesOpened)
            {
                _RenderTreeRecursive(_sourceDirectory);
                ImGui::TreePop();
            }
        }

        ImGui::PopID();
    }

    void ContentBrowser::_RenderTreeRecursive(const std::filesystem::path& directory)
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            if (entry.is_directory() && entry.path().extension() != ".meta")
            {
                const auto& path = entry.path();

                auto u8name = path.filename().u8string();
                std::string name(u8name.begin(), u8name.end());
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth |
                                           ImGuiTreeNodeFlags_DrawLinesFull;
                if (path == _currentDirectory)
                    flags |= ImGuiTreeNodeFlags_Selected;

                bool opened = ImGui::TreeNodeEx(name.c_str(), flags);

                if (ImGui::BeginDragDropSource())
                {
                    std::filesystem::path relativePath = std::filesystem::relative(path, _projectInfo.GetProjectDir());
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload(
                        "CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                    {
                        const wchar_t* relPath = (const wchar_t*)payload->Data;
                        std::filesystem::path sourcePath =
                            std::filesystem::path(_projectInfo.GetProjectDir()) / relPath;
                        _MoveAsset(sourcePath, path);
                    }
                    ImGui::EndDragDropTarget();
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Open"))
                    {
                        _currentDirectory = path;
                        _RefreshAssetList();
                    }
                    if (ImGui::MenuItem("Show in Explorer"))
                        _ShowInExplorer(path);
                    ImGui::Separator();
                    if (ImGui::MenuItem("Rename"))
                    {
                        _contextMenuPath = path;
                        strncpy_s(_renameBuffer, path.filename().string().c_str(), sizeof(_renameBuffer));
                        _showRenameModal = true;
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        _contextMenuPath = path;
                        _showDeleteModal = true;
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                {
                    _currentDirectory = path;
                    _RefreshAssetList();
                }

                if (opened)
                {
                    _RenderTreeRecursive(path);
                    ImGui::TreePop();
                }
            }
        }
    }

    void ContentBrowser::_RenderArea()
    {
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / (_thumbnailSize + _padding));
        if (columnCount < 1)
            columnCount = 1;

        if (ImGui::BeginTable("AssetGrid", columnCount))
        {
            for (const auto& file : _currentDirCache)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(file.Name.c_str());

                // Icon
                auto icon = _iconManager->GetIcon(file.Path, file.IsDirectory);
                ImTextureID textureID = (ImTextureID)((icon) ? icon->GetRendererID() : nullptr);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton("##AssetIcon", textureID, { _thumbnailSize, _thumbnailSize }, { 0, 0 }, { 1, 1 });

                // Drag & Drop
                if (ImGui::BeginDragDropSource())
                {
                    std::filesystem::path relativePath =
                        std::filesystem::relative(file.Path, _projectInfo.GetProjectDir());
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload(
                        "CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

                    ImGui::Image(textureID, { 32, 32 });
                    ImGui::Text("%s", file.Name.c_str());
                    ImGui::EndDragDropSource();
                }

                if (file.IsDirectory)
                {
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                        {
                            const wchar_t* relPath = (const wchar_t*)payload->Data;
                            std::filesystem::path sourcePath =
                                std::filesystem::path(_projectInfo.GetProjectDir()) / relPath;
                            _MoveAsset(sourcePath, file.Path);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }

                ImGui::PopStyleColor();

                // Actions
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (file.IsDirectory)
                    {
                        _currentDirectory /= file.Name;
                        _dirty = true;
                    }
                    else
                    {
                        auto editorLayer = EditorApp::Get().GetEditorLayer();
                        if (file.Extension == ".prefab" && editorLayer)
                        {
                            editorLayer->OpenPrefab(file.Path);
                        }
                        else if (_IsModelFormat(file.Extension) && editorLayer)
                        {
                            editorLayer->OpenMeshPreview(file.Path);
                        }
                        else
                        {
                            _OpenFileInExternalEditor(file.Path);
                        }
                    }
                }

                // Context Menu
                if (ImGui::BeginPopupContextItem())
                {
                    if (ImGui::MenuItem("Open"))
                    {
                        if (file.IsDirectory)
                        {
                            _currentDirectory /= file.Name;
                            _dirty = true;
                        }
                        else
                        {
                            _OpenFileInExternalEditor(file.Path);
                        }
                    }
                    if (ImGui::MenuItem("Show in Explorer"))
                    {
                        _ShowInExplorer(file.Path);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Rename"))
                    {
                        _contextMenuPath = file.Path;
                        strncpy_s(_renameBuffer, file.Name.c_str(), sizeof(_renameBuffer));
                        _showRenameModal = true;
                    }
                    if (ImGui::MenuItem("Delete"))
                    {
                        _contextMenuPath = file.Path;
                        _showDeleteModal = true;
                    }
                    ImGui::EndPopup();
                }

                ImGui::TextWrapped("%s", file.Name.c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->Rect(), ImGui::GetID("ContentBrowserArea")))
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* relPath = (const wchar_t*)payload->Data;
                std::filesystem::path sourcePath = std::filesystem::path(_projectInfo.GetProjectDir()) / relPath;
                _MoveAsset(sourcePath, _currentDirectory);
            }
            ImGui::EndDragDropTarget();
        }

        // Global Context Menu
        if (ImGui::BeginPopupContextWindow("ContentBrowserGlobalCtx",
                                           ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Prefab"))
            {
                _showCreatePrefabPopup = true;
                memset(_newPrefabNameBuffer, 0, sizeof(_newPrefabNameBuffer));
                strcpy_s(_newPrefabNameBuffer, "NewPrefab");
            }
            if (ImGui::MenuItem("Create Folder"))
            {
                _showCreateFolderModal = true;
                strcpy_s(_createFolderBuffer, "NewFolder");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show in Explorer"))
            {
                _ShowInExplorer(_currentDirectory);
            }
            ImGui::EndPopup();
        }
    }

    // Actions and Modals
    void ContentBrowser::_RenderModals()
    {
        // Prefab
        if (_showCreatePrefabPopup)
        {
            ImGui::OpenPopup("Create New Prefab");
        }

        if (ImGui::BeginPopupModal("Create New Prefab", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter Prefab Name:");
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            bool enter = ImGui::InputText(
                "##prefab", _newPrefabNameBuffer, sizeof(_newPrefabNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Separator();

            if (ImGui::Button("Create") || enter)
            {
                if (strlen(_newPrefabNameBuffer) > 0)
                    _CreatePrefab(_newPrefabNameBuffer);
                _showCreatePrefabPopup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                _showCreatePrefabPopup = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Rename
        if (_showRenameModal)
        {
            ImGui::OpenPopup("Rename Asset");
        }
        if (ImGui::BeginPopupModal("Rename Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter new name:");
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            bool enter = ImGui::InputText(
                "##rename", _renameBuffer, sizeof(_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Separator();

            if (ImGui::Button("Rename") || enter)
            {
                _RenameItem(_contextMenuPath, _renameBuffer);
                _showRenameModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                _showRenameModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Delete
        if (_showDeleteModal)
        {
            ImGui::OpenPopup("Delete Asset");
        }

        if (ImGui::BeginPopupModal("Delete Asset", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "Warning: Irreversible action.");
            ImGui::Text("Delete %s ?", _contextMenuPath.filename().string().c_str());
            ImGui::Separator();

            if (ImGui::Button("Yes, Delete"))
            {
                _DeleteItem(_contextMenuPath);
                _showDeleteModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                _showDeleteModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Create
        if (_showCreateFolderModal)
        {
            ImGui::OpenPopup("Create Folder");
        }

        if (ImGui::BeginPopupModal("Create Folder", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Folder Name:");
            if (ImGui::IsWindowAppearing())
            {
                ImGui::SetKeyboardFocusHere();
            }
            bool enter = ImGui::InputText(
                "##folder", _createFolderBuffer, sizeof(_createFolderBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::Separator();

            if (ImGui::Button("Create") || enter)
            {
                _CreateFolder(_createFolderBuffer);
                _showCreateFolderModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();

            if (ImGui::Button("Cancel"))
            {
                _showCreateFolderModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void ContentBrowser::_CreatePrefab(const std::string& name)
    {
        Frost::Scene tempScene;
        auto go = tempScene.CreateGameObject(name);
        std::filesystem::path path = _currentDirectory / name;
        if (path.extension() != ".prefab")
        {
            path += ".prefab";
        }
        Frost::PrefabSerializer::CreatePrefab(go, path);
        _RefreshAssetList();
    }

    void ContentBrowser::_CreateFolder(const std::string& name)
    {
        std::filesystem::path path = _currentDirectory / name;
        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directory(path);
        }
        _RefreshAssetList();
    }

    void ContentBrowser::_RenameItem(const std::filesystem::path& oldPath, const std::string& newName)
    {
        std::filesystem::path newPath = oldPath.parent_path() / newName;
        if (!std::filesystem::is_directory(oldPath) && !newPath.has_extension() && oldPath.has_extension())
        {
            newPath.replace_extension(oldPath.extension());
        }

        try
        {
            if (!std::filesystem::exists(newPath))
            {
                std::filesystem::rename(oldPath, newPath);

                // Rename meta
                std::filesystem::path oldMeta = oldPath;
                oldMeta += ".meta";
                std::filesystem::path newMeta = newPath;
                newMeta += ".meta";
                if (std::filesystem::exists(oldMeta))
                    std::filesystem::rename(oldMeta, newMeta);
            }
        }
        catch (...)
        {
        }

        _RefreshAssetList();
    }

    void ContentBrowser::_DeleteItem(const std::filesystem::path& path)
    {
        try
        {
            std::filesystem::remove_all(path);
            std::filesystem::path meta = path;
            meta += ".meta";
            if (std::filesystem::exists(meta))
            {
                std::filesystem::remove(meta);
            }
        }
        catch (...)
        {
        }
        _RefreshAssetList();
    }

    void ContentBrowser::_MoveAsset(const std::filesystem::path& sourcePath, const std::filesystem::path& destDir)
    {
        if (!std::filesystem::exists(sourcePath) || !std::filesystem::exists(destDir))
        {
            return;
        }
        if (sourcePath.parent_path() == destDir)
        {
            return;
        }

        if (std::filesystem::is_directory(sourcePath))
        {
            auto rel = std::filesystem::relative(destDir, sourcePath);
            if (!rel.empty() && rel.native()[0] != '.')
            {
                return;
            }
        }

        std::filesystem::path destPath = destDir / sourcePath.filename();

        try
        {
            if (!std::filesystem::exists(destPath))
            {
                std::filesystem::rename(sourcePath, destPath);

                // Move associated .meta file if it exists
                std::filesystem::path sourceMeta = sourcePath;
                sourceMeta += ".meta";
                std::filesystem::path destMeta = destPath;
                destMeta += ".meta";

                if (std::filesystem::exists(sourceMeta))
                {
                    std::filesystem::rename(sourceMeta, destMeta);
                }
            }
            else
            {
                FT_ENGINE_CRITICAL("Destination path already exists: {}", destPath.string());
            }
        }
        catch (const std::exception& e)
        {
        }

        _RefreshAssetList();
    }

    void ContentBrowser::_ShowInExplorer(const std::filesystem::path& path)
    {
        if (std::filesystem::is_directory(path))
        {
            ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOW);
        }
        else
        {
            std::string params = "/select,\"" + path.string() + "\"";
            ShellExecuteA(NULL, "open", "explorer.exe", params.c_str(), NULL, SW_SHOW);
        }
    }

    void ContentBrowser::_OpenFileInExternalEditor(const std::filesystem::path& path)
    {
        ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOW);
    }

    bool ContentBrowser::_IsModelFormat(const std::string& extension)
    {
        return std::find(MESH_FILE_EXTENSIONS.begin(), MESH_FILE_EXTENSIONS.end(), extension) !=
               MESH_FILE_EXTENSIONS.end();
    }

} // namespace Editor