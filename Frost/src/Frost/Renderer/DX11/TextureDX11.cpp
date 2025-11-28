#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/DX11/FormatDX11.h"

#define STB_IMAGE_IMPLEMENTATION
#include <assimp/scene.h>
#include <assimp/types.h>
#include <stb_image.h>

#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
    static bool IsDepthFormat(Format format)
    {
        return format == Format::D24_UNORM_S8_UINT;
    }

    TextureDX11::TextureDX11(TextureConfig& config) : Texture(config)
    {
        if (config.layout == TextureLayout::CUBEMAP)
        {
            _CreateCubemap(config);
            return;
        }

        int width = config.width;
        int height = config.height;
        int channels = config.channels;
        std::vector<uint8_t> imageData;

        if (config.aTexture)
        {
            FT_ENGINE_INFO("Loading embedded texture: {}", config.debugName);

            if (config.aTexture->mHeight == 0)
            {
                stbi_uc* data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(config.aTexture->pcData),
                                                      config.aTexture->mWidth,
                                                      &width,
                                                      &height,
                                                      &channels,
                                                      STBI_rgb_alpha);
                FT_ENGINE_ASSERT(data, "Failed to load COMPRESSED embedded texture from memory");

                if (config.format == Format::UNKNOWN)
                {
                    config.format = Format::RGBA8_UNORM;
                    channels = 4;
                }

                if (data)
                {
                    size_t imageSize = (size_t)width * height * 4;
                    imageData.assign(data, data + imageSize);
                    stbi_image_free(data);
                }
            }
            else
            {
                width = config.aTexture->mWidth;
                height = config.aTexture->mHeight;
                FT_ENGINE_ASSERT(width > 0 && height > 0, "Invalid dimensions for UNCOMPRESSED embedded texture");

                size_t numPixels = (size_t)width * height;
                imageData.resize(numPixels * 4);

                const aiTexel* sourceData = reinterpret_cast<const aiTexel*>(config.aTexture->pcData);
                uint8_t* destData = imageData.data();

                for (size_t i = 0; i < numPixels; ++i)
                {
                    destData[i * 4 + 0] = sourceData[i].r;
                    destData[i * 4 + 1] = sourceData[i].g;
                    destData[i * 4 + 2] = sourceData[i].b;
                    destData[i * 4 + 3] = sourceData[i].a;
                }
            }
        }
        else if (!config.path.empty())
        {
            FT_ENGINE_INFO("Loading texture from file: {}", config.path);
            stbi_uc* data = stbi_load(config.path.c_str(), &width, &height, &channels, 0);
            FT_ENGINE_ASSERT(data, "Failed to load texture from file");

            if (config.format == Format::UNKNOWN)
            {
                switch (channels)
                {
                    case 1:
                        config.format = Format::R8_UNORM;
                        break;
                    case 2:
                        config.format = Format::RG8_UNORM;
                        break;
                    case 3:
                        config.format = Format::RGBA8_UNORM;
                        break;
                    case 4:
                    default:
                        config.format = Format::RGBA8_UNORM;
                        break;
                }
            }

            if (data)
            {
                size_t imageSize = (size_t)width * height * channels;
                imageData.assign(data, data + imageSize);
                stbi_image_free(data);
            }

            if (channels == 3)
            {
                std::vector<uint8_t> rgbaData;
                rgbaData.reserve(width * height * 4);

                for (size_t i = 0; i < imageData.size(); i += 3)
                {
                    rgbaData.push_back(imageData[i + 0]); // R
                    rgbaData.push_back(imageData[i + 1]); // G
                    rgbaData.push_back(imageData[i + 2]); // B
                    rgbaData.push_back(255);              // A = opaque
                }

                imageData.swap(rgbaData);
                channels = 4;
            }
        }

        DXGI_FORMAT dxgiFormat = ToDXGIFormat(config.format);
        FT_ENGINE_ASSERT(dxgiFormat != DXGI_FORMAT_UNKNOWN, "Unsupported texture format provided.");

        FT_ENGINE_ASSERT(!(imageData.empty() && (width == 0 || height == 0)),
                         "Texture creation failed: No data loaded and no "
                         "dimensions specified.");

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.ArraySize = 1;
        textureDesc.Format = dxgiFormat;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.MiscFlags = 0;

        bool generateMips = config.hasMipmaps && config.isShaderResource && !imageData.empty();

        UINT bindFlags = 0;
        if (config.isShaderResource)
        {
            bindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (config.isRenderTarget)
        {
            bindFlags |= IsDepthFormat(config.format) ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
        }

        if (generateMips)
        {
            bindFlags |= D3D11_BIND_RENDER_TARGET;
        }
        textureDesc.BindFlags = bindFlags;

        if (generateMips)
        {
            textureDesc.Usage = D3D11_USAGE_DEFAULT;
            textureDesc.CPUAccessFlags = 0;
            textureDesc.MipLevels = 0;
            textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
        else
        {
            textureDesc.MipLevels = 1;
            textureDesc.Usage = !imageData.empty() ? D3D11_USAGE_IMMUTABLE : D3D11_USAGE_DEFAULT;
            textureDesc.CPUAccessFlags = 0;
        }

        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = renderer->GetDevice();
        ID3D11DeviceContext* context = renderer->GetDeviceContext();

        HRESULT hr;

        if (generateMips)
        {
            hr = device->CreateTexture2D(&textureDesc, nullptr, _texture.GetAddressOf());
            FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 Texture2D for mipmap generation.");
        }
        else if (!imageData.empty())
        {
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = imageData.data();
            initData.SysMemPitch = width * GetFormatSize(config.format);
            hr = device->CreateTexture2D(&textureDesc, &initData, _texture.GetAddressOf());
        }
        else
        {
            hr = device->CreateTexture2D(&textureDesc, nullptr, _texture.GetAddressOf());
        }

        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 Texture2D.");

        if (SUCCEEDED(hr))
        {
            if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = dxgiFormat;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip = 0;
                srvDesc.Texture2D.MipLevels = (textureDesc.MipLevels == 0) ? -1 : 1;
                hr = device->CreateShaderResourceView(_texture.Get(), &srvDesc, _srv.GetAddressOf());
                FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create SRV.");
            }
            if (bindFlags & D3D11_BIND_RENDER_TARGET)
            {
                hr = device->CreateRenderTargetView(_texture.Get(), nullptr, _rtv.GetAddressOf());
                FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create RTV.");
            }
            if (bindFlags & D3D11_BIND_DEPTH_STENCIL)
            {
                hr = device->CreateDepthStencilView(_texture.Get(), nullptr, _dsv.GetAddressOf());
                FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create DSV.");
            }
        }

        if (generateMips)
        {
            UINT rowPitch = width * GetFormatSize(config.format);
            context->UpdateSubresource(_texture.Get(), 0, nullptr, imageData.data(), rowPitch, 0);

            if (_srv)
            {
                context->GenerateMips(_srv.Get());
            }
        }

        _config.width = width;
        _config.height = height;
        _config.channels = channels;
        _config.format = config.format;
    }

    TextureDX11::TextureDX11(ID3D11Texture2D* existingTexture, ID3D11RenderTargetView* rtv) :
        Texture(
            [&]()
            {
                FT_ENGINE_ASSERT(existingTexture, "Existing texture cannot be null!");
                D3D11_TEXTURE2D_DESC desc;
                existingTexture->GetDesc(&desc);

                TextureConfig config;
                config.width = desc.Width;
                config.height = desc.Height;
                config.isRenderTarget = true;
                config.isShaderResource = false;
                config.debugName = "BackBufferTexture";
                return config;
            }())
    {
        FT_ENGINE_ASSERT(rtv, "Render target view cannot be null!");

        _texture = existingTexture;
        _rtv = rtv;
        _srv = nullptr;
        _dsv = nullptr;

        D3D11_TEXTURE2D_DESC desc;
        _texture->GetDesc(&desc);
        _config.width = desc.Width;
        _config.height = desc.Height;
        _config.isRenderTarget = true;
    }

    TextureDX11::TextureDX11(ID3D11Texture2D* existingTexture, ID3D11DepthStencilView* dsv) :
        Texture(
            [&]()
            {
                FT_ENGINE_ASSERT(existingTexture, "Existing texture cannot be null!");
                D3D11_TEXTURE2D_DESC desc;
                existingTexture->GetDesc(&desc);

                TextureConfig config;
                config.width = desc.Width;
                config.height = desc.Height;
                config.isRenderTarget = false;
                config.isShaderResource = false;
                config.debugName = "DepthBufferTexture";
                return config;
            }())
    {
        FT_ENGINE_ASSERT(dsv, "Depth stencil view cannot be null for a depth texture!");

        _texture = existingTexture;
        _dsv = dsv;
        _rtv = nullptr;
        _srv = nullptr;

        D3D11_TEXTURE2D_DESC desc;
        _texture->GetDesc(&desc);
        _config.width = desc.Width;
        _config.height = desc.Height;
        _config.isRenderTarget = false;
    }

    TextureDX11::TextureDX11(uint32_t width,
                             uint32_t height,
                             Format format,
                             const void* pixelData,
                             const std::string& debugName) :
        Texture(
            [&]()
            {
                TextureConfig config;
                config.width = width;
                config.height = height;
                config.format = format;
                config.isRenderTarget = false;
                config.isShaderResource = true;
                config.hasMipmaps = false;
                config.debugName = debugName;
                return config;
            }())
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = renderer->GetDevice();

        DXGI_FORMAT dxgiFormat = ToDXGIFormat(format);
        FT_ENGINE_ASSERT(dxgiFormat != DXGI_FORMAT_UNKNOWN, "Unsupported texture format provided.");

        uint32_t bytesPerPixel = GetFormatSize(format);
        uint32_t rowPitch = width * bytesPerPixel;

        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = dxgiFormat;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = pixelData;
        initData.SysMemPitch = rowPitch;
        initData.SysMemSlicePitch = rowPitch * height;

        HRESULT hr = device->CreateTexture2D(&textureDesc, &initData, _texture.GetAddressOf());
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 Texture2D from pixel data.");

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = dxgiFormat;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;

        hr = device->CreateShaderResourceView(_texture.Get(), &srvDesc, _srv.GetAddressOf());
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create SRV for texture from pixel data.");
    }

    const std::vector<uint8_t> TextureDX11::GetData() const
    {
        if (_dataCached)
        {
            return _dataCache;
        }

        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11DeviceContext* context = renderer->GetDeviceContext();
        ID3D11Device* device = renderer->GetDevice();
        FT_ENGINE_ASSERT(context, "Device context is null.");
        FT_ENGINE_ASSERT(device, "Device is null.");

        D3D11_TEXTURE2D_DESC desc;
        _texture->GetDesc(&desc);

        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT hr = device->CreateTexture2D(&desc, nullptr, stagingTexture.GetAddressOf());
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create staging texture.");

        context->CopyResource(stagingTexture.Get(), _texture.Get());

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to map staging texture.");

        const Format textureFormat = _config.format;
        const uint32_t bytesPerPixel = Frost::GetFormatSize(textureFormat);

        size_t dataSize = (size_t)desc.Width * desc.Height * bytesPerPixel;
        _dataCache.resize(dataSize);

        const uint32_t bytesPerRow = desc.Width * bytesPerPixel;
        const uint8_t* src = static_cast<const uint8_t*>(mappedResource.pData);
        uint8_t* dst = _dataCache.data();

        for (uint32_t y = 0; y < desc.Height; ++y)
        {
            std::memcpy(dst, src, bytesPerRow);
            src += mappedResource.RowPitch;
            dst += bytesPerRow;
        }

        context->Unmap(stagingTexture.Get(), 0);
        _dataCached = true;
        return _dataCache;
    }

    void TextureDX11::Bind(Slot slot) const
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11DeviceContext* context = renderer->GetDeviceContext();
        FT_ENGINE_ASSERT(_srv, "Cannot bind texture: SRV is null.");
        context->PSSetShaderResources(static_cast<UINT>(slot), 1, _srv.GetAddressOf());
    }

    struct FaceMap
    {
        int x;
        int y;
    };
    const FaceMap CROSS_FACE_COORDS[6] = {
        { 2, 1 }, // 0: +X (Right)
        { 0, 1 }, // 1: -X (Left)
        { 1, 0 }, // 2: +Y (Top)
        { 1, 2 }, // 3: -Y (Bottom)
        { 1, 1 }, // 4: +Z (Front)
        { 3, 1 }  // 5: -Z (Back)
    };

    void TextureDX11::_CreateCubemap(TextureConfig& config)
    {
        // 1. Initialisation
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = renderer->GetDevice();
        FT_ENGINE_ASSERT(device, "D3D11 Device is null.");

        int width = 0, height = 0, channels = 0;
        std::vector<stbi_uc*> loadedData(6, nullptr);
        int faceWidth = 0;
        int faceHeight = 0;

        bool loadSuccess = true;

        if (config.isUnfoldedCubemap)
        {
            // Cubemap from unfolded texture
            FT_ENGINE_INFO("Loading unfolded cubemap from file: {}", config.path);

            stbi_uc* unfoldedData = stbi_load(config.path.c_str(), &width, &height, &channels, 4);
            if (!unfoldedData)
            {
                FT_ENGINE_ERROR("Failed to load unfolded cubemap texture: {0}", config.path);
                return;
            }

            if (width % 6 == 0 && width / 6 == height)
            {
                faceWidth = width / 6;
                faceHeight = height;

                for (int i = 0; i < 6; ++i)
                {
                    FaceMap stripCoords[] = { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 } };

                    int srcX = stripCoords[i].x * faceWidth;
                    int srcY = stripCoords[i].y * faceHeight;
                    int bytesPerPixel = 4;
                    int faceRowPitch = faceWidth * bytesPerPixel;
                    size_t faceDataSize = (size_t)faceWidth * faceHeight * bytesPerPixel;

                    stbi_uc* faceData = new stbi_uc[faceDataSize];
                    loadedData[i] = faceData;

                    int totalImageRowPitch = width * bytesPerPixel;
                    for (int y = 0; y < faceHeight; ++y)
                    {
                        int srcRowOffset = (srcY + y) * totalImageRowPitch + srcX * bytesPerPixel;
                        const stbi_uc* src = unfoldedData + srcRowOffset;
                        stbi_uc* dst = faceData + (y * faceRowPitch);
                        std::memcpy(dst, src, faceRowPitch);
                    }
                }
            }
            else if ((width % 4 == 0 && width / 4 == height / 3) || (width % 3 == 0 && width / 3 == height / 4))
            {
                // Format Cross 4x3 or 3x4 (W=4*H/3 ou W=3*H/4)
                faceWidth = width / 4;
                faceHeight = height / 3;
                if (width % 3 == 0 && width / 3 == height / 4)
                {
                    faceWidth = width / 3;
                    faceHeight = height / 4;
                }
                FT_ENGINE_ASSERT(faceWidth == faceHeight, "Unfolded cubemap face dimensions must be square.");

                // Cross 4x3
                for (int i = 0; i < 6; ++i)
                {
                    int srcX = CROSS_FACE_COORDS[i].x * faceWidth;
                    int srcY = CROSS_FACE_COORDS[i].y * faceHeight;

                    if (srcX + faceWidth > width || srcY + faceHeight > height)
                    {
                        FT_ENGINE_ERROR(
                            "Cubemap face coordinates ({}, {}) are outside the bounds of the image.", srcX, srcY);
                        loadSuccess = false;
                        break;
                    }

                    int bytesPerPixel = 4;
                    int faceRowPitch = faceWidth * bytesPerPixel;
                    size_t faceDataSize = (size_t)faceWidth * faceHeight * bytesPerPixel;

                    stbi_uc* faceData = new stbi_uc[faceDataSize];
                    loadedData[i] = faceData;

                    int totalImageRowPitch = width * bytesPerPixel;

                    for (int y = 0; y < faceHeight; ++y)
                    {
                        int srcRowOffset = (srcY + y) * totalImageRowPitch + srcX * bytesPerPixel;
                        const stbi_uc* src = unfoldedData + srcRowOffset;

                        stbi_uc* dst = faceData + (y * faceRowPitch);

                        std::memcpy(dst, src, faceRowPitch);
                    }
                }
            }
            else
            {
                FT_ENGINE_ERROR("Unfolded cubemap dimensions ({0}x{1}) do not match standard 6x1, 1x6, 3x4, or 4x3 "
                                "aspect ratio. Failed to determine face size.",
                                width,
                                height);
                loadSuccess = false;
            }

            stbi_image_free(unfoldedData);

            if (!loadSuccess)
            {
                for (int i = 0; i < 6; ++i)
                {
                    if (loadedData[i])
                    {
                        delete[] loadedData[i];
                    }
                }

                return;
            }
        }
        else
        {
            // 6 faces
            FT_ENGINE_ASSERT(config.faceFilePaths.size() == 6, "A cubemap requires exactly 6 texture faces.");
            if (config.faceFilePaths.size() != 6)
                return;

            for (int i = 0; i < 6; ++i)
            {
                loadedData[i] = stbi_load(config.faceFilePaths[i].c_str(), &width, &height, &channels, 4);
                if (!loadedData[i])
                {
                    FT_ENGINE_ERROR("Failed to load cubemap face: {0}", config.faceFilePaths[i]);
                    loadSuccess = false;
                    for (int j = 0; j < i; ++j)
                    {
                        stbi_image_free(loadedData[j]);
                    }
                    return;
                }
            }
            faceWidth = width;
            faceHeight = height;
        }

        _config.width = faceWidth;
        _config.height = faceHeight;
        _config.format = Format::RGBA8_UNORM;

        // Ressource Creation
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = faceWidth;
        textureDesc.Height = faceHeight;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 6;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        D3D11_SUBRESOURCE_DATA subresourceData[6];
        for (int i = 0; i < 6; ++i)
        {
            subresourceData[i].pSysMem = loadedData[i];
            subresourceData[i].SysMemPitch = faceWidth * 4;
            subresourceData[i].SysMemSlicePitch = 0;
        }

        HRESULT hr = device->CreateTexture2D(&textureDesc, subresourceData, _texture.GetAddressOf());

        // Cleanup
        if (config.isUnfoldedCubemap)
        {
            for (int i = 0; i < 6; ++i)
            {
                if (loadedData[i])
                {
                    delete[] loadedData[i];
                }
            }
        }
        else
        {
            for (int i = 0; i < 6; ++i)
            {
                if (loadedData[i])
                {
                    stbi_image_free(loadedData[i]);
                }
            }
        }

        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 cubemap texture resource.");
        if (FAILED(hr))
            return;

        // SRV Creation
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = textureDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = 1;

        hr = device->CreateShaderResourceView(_texture.Get(), &srvDesc, _srv.GetAddressOf());
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 cubemap SRV.");
    }
} // namespace Frost