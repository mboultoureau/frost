#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Renderer/Format.h"
#include "Frost/Renderer/GPUResource.h"
#include "Frost/Renderer/Renderer.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

struct aiTexture;

namespace Frost
{
    enum TextureType
    {
        // Related to assimp texture types
        NONE = 0,
        DIFFUSE = 1,
        SPECULAR = 2,
        AMBIENT = 3,
        EMISSIVE = 4,
        HEIGHT = 5,
        NORMALS = 6,
        SHININESS = 7,
        OPACITY = 8,
        DISPLACEMENT = 9,
        LIGHTMAP = 10,
        REFLECTION = 11,

        // PBR
        BASE_COLOR = 12,
        NORMAL_CAMERA = 13,
        EMISSION_COLOR = 14,
        METALNESS = 15,
        DIFFUSE_ROUGHNESS = 16,
        AMBIENT_OCCLUSION = 17,

        UNKNOWN = 18,
        SHEEN = 19,
        CLEARCOAT = 20,
        TRANSMISSION = 21,

        // DirectX specific
        SHADOW = 100,
        HUD = 101,
        HEIGHTMAP = 102,
        BILLBOARD = 103
    };

    enum TextureChannel
    {
        R = 0,
        G = 1,
        B = 2,
        A = 3
    };

    enum class TextureLayout
    {
        TEXTURE_2D,
        CUBEMAP
    };

    struct TextureConfig
    {
        TextureType textureType = TextureType::NONE;
        std::vector<uint8_t> fileData;
        Asset::Path path = "";
        Asset::DebugName debugName = "Texture";
        Format format = Format::UNKNOWN;

        uint32_t channels = 4;
        uint32_t width = 0;
        uint32_t height = 0;

        bool isRenderTarget = false;
        bool isShaderResource = true;
        bool hasMipmaps = true;
        bool isCompressed = false;

        TextureLayout layout = TextureLayout::TEXTURE_2D;
        std::array<std::string, 6> faceFilePaths;
        bool isUnfoldedCubemap = false;
        bool loadImmediately = true;
    };

    class Texture : public Asset, GPUResource
    {
    public:
        using Slot = unsigned int;

        Texture(const TextureConfig& config) : _config(config) {}
        virtual ~Texture() = default;

        // Async API
        virtual void LoadCPU(const std::string& path, const TextureConfig& config) = 0;
        virtual void UploadGPU() = 0;

        static std::shared_ptr<Texture> Create(TextureConfig& config);

        const std::string& GetPath() const noexcept { return _config.path; }
        const TextureType GetTextureType() const noexcept { return _config.textureType; }
        const Format GetFormat() const noexcept { return _config.format; }

        const uint32_t GetChannels() const { return _config.channels; }
        const uint32_t GetHeight() const { return _config.height; }
        const uint32_t GetWidth() const { return _config.width; }

        virtual const std::vector<uint8_t> GetData() const = 0;
        virtual void Bind(Slot slot) const = 0;

        virtual void* GetRendererID() const = 0;

        virtual bool SaveToFile(const std::string& path) const = 0;

    protected:
        TextureConfig _config;
        mutable std::vector<uint8_t> _dataCache;
        mutable bool _dataCached = false;
    };
} // namespace Frost
