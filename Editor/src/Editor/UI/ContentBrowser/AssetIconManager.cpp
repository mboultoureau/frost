#include "Editor/UI/ContentBrowser/AssetIconManager.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Asset/MeshConfig.h"
#include "Editor/EditorApp.h"

#include <thread>

#undef max

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

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

        auto _projectFolder = EditorApp::Get().GetProjectDirectory();
        _thumbnailCacheDir = _projectFolder / ".frost" / "thumbnails";
        if (!std::filesystem::exists(_thumbnailCacheDir))
        {
            std::filesystem::create_directories(_thumbnailCacheDir);
        }
    }

    std::filesystem::path AssetIconManager::_GetCachePath(const std::filesystem::path& assetPath)
    {
        std::string pathStr = std::filesystem::absolute(assetPath).string();
        size_t hash = std::hash<std::string>{}(pathStr);
        return _thumbnailCacheDir / (std::to_string(hash) + ".png");
    }

    void AssetIconManager::Update()
    {
        if (!_loadQueue.empty())
        {
            std::filesystem::path assetPath = _loadQueue.front();
            _loadQueue.pop_front();

            std::string pathKey = assetPath.string();
            std::shared_ptr<Frost::Texture> newIcon = nullptr;
            std::string ext = assetPath.extension().string();

            if (ext == ".png" || ext == ".jpg" || ext == ".dds" || ext == ".tga")
            {
                Frost::TextureConfig config;
                config.path = assetPath.string();
                config.debugName = pathKey;
                newIcon = Frost::Texture::Create(config);
            }
            else if (_IsModelFormat(ext))
            {
                std::filesystem::path cachePath = _GetCachePath(assetPath);
                bool cacheValid = false;

                // Cache for performance
                if (std::filesystem::exists(cachePath))
                {
                    auto assetTime = std::filesystem::last_write_time(assetPath);
                    auto cacheTime = std::filesystem::last_write_time(cachePath);

                    if (cacheTime >= assetTime)
                    {
                        Frost::TextureConfig config;
                        config.path = cachePath.string();
                        config.debugName = "ThumbCache_" + assetPath.stem().string();
                        newIcon = Frost::Texture::Create(config);
                        cacheValid = true;
                    }
                }

                if (!cacheValid)
                {
                    auto renderTex = _GenerateModelThumbnail(assetPath);
                    if (renderTex)
                    {
                        renderTex->SaveToFile(cachePath.string());

                        Frost::TextureConfig config;
                        config.path = cachePath.string();
                        newIcon = Frost::Texture::Create(config);

                        // Save memory by pruning unused assets
                        Frost::AssetManager::PruneUnused();
                    }
                }
            }

            if (newIcon)
            {
                try
                {
                    auto time = std::filesystem::last_write_time(assetPath);
                    _iconCache[pathKey] = { newIcon, time };
                }
                catch (...)
                {
                    _iconCache[pathKey] = { newIcon, std::filesystem::file_time_type() };
                }
            }

            _pendingPaths.erase(pathKey);
        }
    }

    std::shared_ptr<Frost::Texture> AssetIconManager::GetIcon(const std::filesystem::path& path, bool isDirectory)
    {
        if (isDirectory)
        {
            return _folderIcon;
        }

        std::string pathKey = path.string();

        if (_iconCache.contains(pathKey))
        {
            return _iconCache.at(pathKey).texture;
        }

        if (_pendingPaths.contains(pathKey))
        {
            return _fileIcon;
        }

        _pendingPaths.insert(pathKey);
        _loadQueue.push_back(path);

        return _fileIcon;
    }

    std::shared_ptr<Frost::Texture> AssetIconManager::_GenerateModelThumbnail(const std::filesystem::path& path)
    {
        uint32_t thumbSize = 256;
        Frost::TextureConfig texConfig = {};
        texConfig.width = thumbSize;
        texConfig.height = thumbSize;
        texConfig.format = Frost::Format::RGBA8_UNORM;
        texConfig.isRenderTarget = true;
        texConfig.isShaderResource = true;
        texConfig.hasMipmaps = false;
        texConfig.debugName = "Thumbnail_" + path.filename().string();

        auto thumbnailTexture = Frost::Texture::Create(texConfig);

        Frost::Scene tempScene("ThumbnailGen");

        auto meshEntity = tempScene.CreateGameObject("Mesh");
        auto& sm = meshEntity.AddComponent<StaticMesh>(MeshSourceFile{ path.string() });
        meshEntity.AddComponent<Transform>();

        if (!sm.GetModel() || !sm.GetModel()->HasMeshes())
        {
            return nullptr;
        }

        auto model = sm.GetModel();
        int maxWaitFrames = 200;
        bool allTexturesReady = false;

        while (!allTexturesReady && maxWaitFrames > 0)
        {
            allTexturesReady = true;
            for (const auto& mat : model->GetMaterials())
            {
                for (const auto& tex : mat.albedoTextures)
                {
                    if (tex)
                    {
                        if (tex->GetStatus() != Frost::AssetStatus::Loaded)
                        {
                            allTexturesReady = false;
                            tex->UploadGPU();
                        }
                    }
                }
                if (!allTexturesReady)
                    break;
            }

            if (!allTexturesReady)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                maxWaitFrames--;
            }
        }

        auto cameraEntity = tempScene.CreateGameObject("Camera");
        auto& camComp = cameraEntity.AddComponent<Camera>();
        camComp.viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
        camComp.nearClip = 0.1f;
        camComp.farClip = 1000.0f;

        auto& camTrans = cameraEntity.AddComponent<Transform>();

        Frost::BoundingBox bounds = sm.GetModel()->GetBoundingBox();
        _FocusCameraOnBounds(camTrans, bounds);

        auto lightEntity = tempScene.CreateGameObject("Light");
        auto& lightComp = lightEntity.AddComponent<Frost::Component::Light>();
        lightComp.type = Frost::Component::LightType::Directional;
        lightComp.intensity = 1.2f;
        lightComp.color = { 1.0f, 0.95f, 0.9f };

        auto& lightTrans = lightEntity.AddComponent<Frost::Component::Transform>();
        lightTrans.Rotate(Frost::Math::EulerAngles{ -45.0f, 45.0f, 0.0f });

        tempScene.SetEditorRenderTarget(thumbnailTexture);
        tempScene.Update(0.016f);
        tempScene.LateUpdate(0.016f);

        return thumbnailTexture;
    }

    void AssetIconManager::_FocusCameraOnBounds(Frost::Component::Transform& cameraTransform,
                                                const Frost::BoundingBox& bounds)
    {
        using namespace Frost::Math;

        Vector3 min = { bounds.min.x, bounds.min.y, bounds.min.z };
        Vector3 max = { bounds.max.x, bounds.max.y, bounds.max.z };

        Vector3 center = (min + max) * 0.5f;

        float sizeX = bounds.max.x - bounds.min.x;
        float sizeY = bounds.max.y - bounds.min.y;
        float sizeZ = bounds.max.z - bounds.min.z;

        float maxDim = std::max({ sizeX, sizeY, sizeZ });
        if (maxDim < 0.1f)
        {
            maxDim = 0.1f;
        }

        float distance = maxDim * 1.2f;

        Vector3 direction = { -1.0f, -1.0f, 1.0f };
        float len = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
        direction = { direction.x / len, direction.y / len, direction.z / len };

        cameraTransform.position = center - (direction * distance);
        cameraTransform.LookAt(center);
    }

    bool AssetIconManager::_IsModelFormat(const std::string& extension)
    {
        return std::find(MESH_FILE_EXTENSIONS.begin(), MESH_FILE_EXTENSIONS.end(), extension) !=
               MESH_FILE_EXTENSIONS.end();
    }

    void AssetIconManager::ClearCache()
    {
        _iconCache.clear();
    }
} // namespace Editor