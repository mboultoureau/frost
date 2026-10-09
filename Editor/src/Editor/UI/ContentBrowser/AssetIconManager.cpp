#include "Editor/UI/ContentBrowser/AssetIconManager.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Asset/MeshConfig.h"
#include "Editor/EditorApp.h"

#include <algorithm>
#include <cctype>

#undef min
#undef max

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_STATIC
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace Editor
{
    static constexpr uint32_t THUMBNAIL_IMAGE_SIZE = 128;
    static constexpr size_t MAX_IMAGE_UPLOADS_PER_FRAME = 4;

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
        std::error_code ec;
        if (!std::filesystem::exists(_thumbnailCacheDir, ec))
        {
            std::filesystem::create_directories(_thumbnailCacheDir, ec);
        }

        _workerThread = std::thread(&AssetIconManager::_WorkerLoop, this);
    }

    AssetIconManager::~AssetIconManager()
    {
        _running = false;
        _workCv.notify_all();
        if (_workerThread.joinable())
        {
            _workerThread.join();
        }
    }

    std::filesystem::path AssetIconManager::_GetCachePath(const std::filesystem::path& assetPath)
    {
        std::error_code ec;
        std::string pathStr = std::filesystem::absolute(assetPath, ec).string();
        size_t hash = std::hash<std::string>{}(pathStr);
        return _thumbnailCacheDir / (std::to_string(hash) + ".png");
    }

    bool AssetIconManager::_IsImageFormat(const std::string& extension)
    {
        std::string ext = extension;
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
        return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp" || ext == ".hdr";
    }

    void AssetIconManager::_WorkerLoop()
    {
        while (_running)
        {
            std::filesystem::path currentPath;
            {
                std::unique_lock<std::mutex> lock(_workMutex);
                _workCv.wait(lock, [this]() { return !_running || !_imageWorkQueue.empty(); });

                if (!_running)
                    break;

                if (_imageWorkQueue.empty())
                    continue;

                currentPath = _imageWorkQueue.front();
                _imageWorkQueue.pop_front();
            }

            Thumbnail thumb;
            if (_ProcessImageThumbnail(currentPath, thumb))
            {
                std::lock_guard<std::mutex> lock(_workMutex);
                _completedThumbnails.push_back(std::move(thumb));
            }
            else
            {
                Thumbnail failedThumb;
                failedThumb.pathKey = currentPath.string();
                failedThumb.width = 0;
                failedThumb.height = 0;
                std::lock_guard<std::mutex> lock(_workMutex);
                _completedThumbnails.push_back(std::move(failedThumb));
            }
        }
    }

    bool AssetIconManager::_ProcessImageThumbnail(const std::filesystem::path& assetPath, Thumbnail& outResult)
    {
        std::error_code ec;
        if (!std::filesystem::exists(assetPath, ec))
            return false;

        auto assetTime = std::filesystem::last_write_time(assetPath, ec);
        if (ec)
            return false;

        std::filesystem::path cachePath = _GetCachePath(assetPath);

        if (std::filesystem::exists(cachePath, ec))
        {
            auto cacheTime = std::filesystem::last_write_time(cachePath, ec);
            if (!ec && cacheTime >= assetTime)
            {
                int w = 0, h = 0, channels = 0;
                stbi_uc* pixels = stbi_load(cachePath.string().c_str(), &w, &h, &channels, 4);
                if (pixels)
                {
                    outResult.pathKey = assetPath.string();
                    outResult.width = static_cast<uint32_t>(w);
                    outResult.height = static_cast<uint32_t>(h);
                    outResult.pixels.assign(pixels, pixels + ((size_t)w * h * 4));
                    outResult.lastWriteTime = assetTime;
                    stbi_image_free(pixels);
                    return true;
                }
            }
        }

        int origW = 0, origH = 0, origChannels = 0;
        stbi_uc* origPixels = stbi_load(assetPath.string().c_str(), &origW, &origH, &origChannels, 4);
        if (!origPixels)
            return false;

        int targetW = origW;
        int targetH = origH;

        if (origW > (int)THUMBNAIL_IMAGE_SIZE || origH > (int)THUMBNAIL_IMAGE_SIZE)
        {
            if (origW > origH)
            {
                targetW = (int)THUMBNAIL_IMAGE_SIZE;
                targetH = std::max(1, (origH * (int)THUMBNAIL_IMAGE_SIZE) / origW);
            }
            else
            {
                targetH = (int)THUMBNAIL_IMAGE_SIZE;
                targetW = std::max(1, (origW * (int)THUMBNAIL_IMAGE_SIZE) / origH);
            }
        }

        std::vector<uint8_t> resizedPixels((size_t)targetW * targetH * 4);
        if (origW == targetW && origH == targetH)
        {
            std::memcpy(resizedPixels.data(), origPixels, (size_t)targetW * targetH * 4);
        }
        else
        {
            stbir_resize_uint8_srgb(origPixels, origW, origH, 0, resizedPixels.data(), targetW, targetH, 0, STBIR_RGBA);
        }
        stbi_image_free(origPixels);

        if (!std::filesystem::exists(_thumbnailCacheDir, ec))
        {
            std::filesystem::create_directories(_thumbnailCacheDir, ec);
        }
        stbi_write_png(cachePath.string().c_str(), targetW, targetH, 4, resizedPixels.data(), targetW * 4);

        outResult.pathKey = assetPath.string();
        outResult.width = static_cast<uint32_t>(targetW);
        outResult.height = static_cast<uint32_t>(targetH);
        outResult.pixels = std::move(resizedPixels);
        outResult.lastWriteTime = assetTime;
        return true;
    }

    void AssetIconManager::Update()
    {
        std::vector<Thumbnail> readyBatch;
        {
            std::lock_guard<std::mutex> lock(_workMutex);
            size_t count = std::min(MAX_IMAGE_UPLOADS_PER_FRAME, _completedThumbnails.size());
            if (count > 0)
            {
                readyBatch.insert(readyBatch.end(),
                                  std::make_move_iterator(_completedThumbnails.begin()),
                                  std::make_move_iterator(_completedThumbnails.begin() + count));
                _completedThumbnails.erase(_completedThumbnails.begin(), _completedThumbnails.begin() + count);
            }
        }

        for (auto& thumb : readyBatch)
        {
            if (thumb.width > 0 && thumb.height > 0 && !thumb.pixels.empty())
            {
                Frost::TextureConfig config;
                config.width = thumb.width;
                config.height = thumb.height;
                config.channels = 4;
                config.format = Frost::Format::RGBA8_UNORM;
                config.fileData = std::move(thumb.pixels);
                config.isCompressed = false;
                config.hasMipmaps = false;
                config.isShaderResource = true;
                config.loadImmediately = true;
                config.debugName = "Thumb_" + thumb.pathKey;

                auto texture = Frost::Texture::Create(config);
                if (texture)
                {
                    _iconCache[thumb.pathKey] = { texture, thumb.lastWriteTime };
                }
            }
            else
            {
                // Fallback to default file icon
                _iconCache[thumb.pathKey] = { _fileIcon, thumb.lastWriteTime };
            }
            _pendingPaths.erase(thumb.pathKey);
        }

        if (!_loadQueue.empty())
        {
            std::filesystem::path assetPath = _loadQueue.front();
            _loadQueue.pop_front();

            std::string pathKey = assetPath.string();
            std::shared_ptr<Frost::Texture> newIcon = nullptr;
            std::string ext = assetPath.extension().string();

            if (_IsModelFormat(ext))
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
            else if (ext == ".dds")
            {
                Frost::TextureConfig config;
                config.path = assetPath.string();
                config.debugName = pathKey;
                newIcon = Frost::Texture::Create(config);
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

        std::string ext = path.extension().string();
        if (_IsImageFormat(ext))
        {
            {
                std::lock_guard<std::mutex> lock(_workMutex);
                _imageWorkQueue.push_back(path);
            }
            _workCv.notify_one();
        }
        else
        {
            _loadQueue.push_back(path);
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
        auto& lightComp = lightEntity.AddComponent<Frost::Component::Light>(LightDirectional{});
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
        {
            std::lock_guard<std::mutex> lock(_workMutex);
            _imageWorkQueue.clear();
            _completedThumbnails.clear();
        }
        _loadQueue.clear();
        _pendingPaths.clear();
    }
} // namespace Editor