#include "Editor/UI/ContentBrowser/AssetIconManager.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Light.h"

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
    }

    std::shared_ptr<Frost::Texture> AssetIconManager::GetIcon(const std::filesystem::path& path, bool isDirectory)
    {
        if (isDirectory)
        {
            return _folderIcon;
        }

        std::string pathKey = path.string();
        std::string ext = path.extension().string();

        // Check last write to see if needs update
        std::filesystem::file_time_type currentWriteTime;
        try
        {
            currentWriteTime = std::filesystem::last_write_time(path);
        }
        catch (...)
        {
            return _fileIcon;
        }

        // Check cache
        if (_iconCache.contains(pathKey))
        {
            const auto& entry = _iconCache.at(pathKey);
            if (entry.lastWriteTime == currentWriteTime)
            {
                return entry.texture;
            }
        }

        std::shared_ptr<Frost::Texture> newIcon = nullptr;

        // If the file is an image, load and cache its thumbnail
        if (ext == ".png" || ext == ".jpg" || ext == ".dds" || ext == ".tga")
        {
            Frost::TextureConfig config;
            config.path = path.string();
            config.debugName = pathKey;
            newIcon = Frost::Texture::Create(config);
        }
        // Model formats
        else if (_IsModelFormat(ext))
        {
            newIcon = _GenerateModelThumbnail(path);
        }

        if (newIcon)
        {
            _iconCache[pathKey] = { newIcon, currentWriteTime };
            return newIcon;
        }

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
        auto& sm = meshEntity.AddComponent<StaticMesh>(path.string());
        meshEntity.AddComponent<Transform>();

        if (!sm.GetModel() || !sm.GetModel()->HasMeshes())
        {
            return nullptr;
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
        static const std::vector<std::string> modelExts = { ".fbx", ".glb", ".gltf", ".obj", ".dae", ".blend" };
        return std::find(modelExts.begin(), modelExts.end(), extension) != modelExts.end();
    }

    void AssetIconManager::ClearCache()
    {
        _iconCache.clear();
    }
} // namespace Editor