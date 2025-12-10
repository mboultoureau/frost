#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/DX11/FormatDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <assimp/texture.h>
#include <filesystem>

namespace Frost
{
    struct FaceMap
    {
        int x;
        int y;
    };

    // Order: +X, -X, +Y, -Y, +Z, -Z
    static const FaceMap CROSS_FACE_COORDS[6] = {
        { 2, 1 }, // +X (Right)
        { 0, 1 }, // -X (Left)
        { 1, 0 }, // +Y (Top)
        { 1, 2 }, // -Y (Bottom)
        { 1, 1 }, // +Z (Front)
        { 3, 1 }  // -Z (Back)
    };

    static bool IsDepthFormat(Format format)
    {
        return format == Format::D24_UNORM_S8_UINT || format == Format::R24G8_TYPELESS;
    }

    TextureDX11::TextureDX11() : Texture({})
    {
        SetStatus(AssetStatus::Unloaded);
    }

    TextureDX11::TextureDX11(TextureConfig& config) : Texture(config)
    {
        _config = config;

        if (_config.loadImmediately)
        {
            LoadCPU(_config.path, _config);
            UploadGPU();
        }
        else
        {
            SetStatus(AssetStatus::Loading);
        }
    }

    TextureDX11::TextureDX11(ID3D11Texture2D* existingTexture, ID3D11RenderTargetView* rtv) : Texture({})
    {
        D3D11_TEXTURE2D_DESC desc;
        existingTexture->GetDesc(&desc);

        _config.width = desc.Width;
        _config.height = desc.Height;
        _config.isRenderTarget = true;
        _config.isShaderResource = false;
        _config.debugName = "BackBufferTexture";

        _texture = existingTexture;
        _rtv = rtv;
        SetStatus(AssetStatus::Loaded);
    }

    TextureDX11::TextureDX11(ID3D11Texture2D* existingTexture, ID3D11DepthStencilView* dsv) : Texture({})
    {
        D3D11_TEXTURE2D_DESC desc;
        existingTexture->GetDesc(&desc);

        _config.width = desc.Width;
        _config.height = desc.Height;
        _config.isRenderTarget = false;
        _config.isShaderResource = false;
        _config.debugName = "DepthBufferTexture";

        _texture = existingTexture;
        _dsv = dsv;
        SetStatus(AssetStatus::Loaded);
    }

    TextureDX11::TextureDX11(uint32_t width,
                             uint32_t height,
                             Format format,
                             const void* pixelData,
                             const std::string& debugName) :
        Texture({})
    {
        _config.width = width;
        _config.height = height;
        _config.format = format;
        _config.isRenderTarget = false;
        _config.isShaderResource = true;
        _config.hasMipmaps = false;
        _config.debugName = debugName;

        if (pixelData)
        {
            size_t size = width * height * GetFormatSize(format);
            _cpuData.resize(size);
            std::memcpy(_cpuData.data(), pixelData, size);
        }

        SetStatus(AssetStatus::Loading);
        UploadGPU();
    }

    void TextureDX11::LoadCPU(const std::string& path, const TextureConfig& config)
    {
        _config = config;

        if (_config.layout == TextureLayout::CUBEMAP)
        {
            _CreateCubemapCPU();
            return;
        }

        int width = 0, height = 0, channels = 0;
        stbi_uc* data = nullptr;
        bool shouldFreeSTB = false;

        if (!_config.fileData.empty())
        {
            if (!_config.isCompressed)
            {
                width = _config.width;
                height = _config.height;

                size_t size = _config.fileData.size();
                _cpuData.resize(size);
                std::memcpy(_cpuData.data(), _config.fileData.data(), size);

                _config.channels = 4;
                _config.format = config.format;
                SetStatus(AssetStatus::Loading);
                return;
            }
            else
            {

                data = stbi_load_from_memory(_config.fileData.data(),
                                             static_cast<int>(_config.fileData.size()),
                                             &width,
                                             &height,
                                             &channels,
                                             STBI_rgb_alpha);

                if (data == nullptr)
                {
                    FT_ENGINE_ERROR("TextureDX11: Failed to load image from memory for {}", _config.debugName);
                    SetStatus(AssetStatus::Failed);
                    return;
                }

                _config.channels = STBI_rgb_alpha;
                _config.width = width;
                _config.height = height;
                _config.format = Format::RGBA8_UNORM;
                shouldFreeSTB = true;
            }
        }
        else if (!_config.path.empty())
        {
            _mmFile = std::make_unique<MemoryMappedFile>(_config.path);
            if (_mmFile->IsValid())
            {
                data = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(_mmFile->GetData()),
                                             static_cast<int>(_mmFile->GetSize()),
                                             &width,
                                             &height,
                                             &channels,
                                             0);

                _config.channels = static_cast<uint32_t>(channels);
                shouldFreeSTB = true;
            }
            else
            {
                FT_ENGINE_ERROR("TextureDX11: Failed to memory map file: {}", _config.path);
                SetStatus(AssetStatus::Failed);
                return;
            }
        }

        if (!data && _cpuData.empty())
        {
            if (_config.isRenderTarget)
            {
                SetStatus(AssetStatus::Loading);
                return;
            }

            FT_ENGINE_ERROR("TextureDX11: No data loaded for {}", _config.debugName);
            SetStatus(AssetStatus::Failed);
            return;
        }

        if (data)
        {
            _config.width = width;
            _config.height = height;

            if (_config.channels == 3)
            {
                _config.format = Format::RGBA8_UNORM;

                size_t numPixels = (size_t)width * height;
                _cpuData.resize(numPixels * 4);

                uint8_t* dest = _cpuData.data();
                const uint8_t* src = data;

                // Conversion RGB -> RGBA
                for (size_t i = 0; i < numPixels; ++i)
                {
                    dest[i * 4 + 0] = src[i * 3 + 0];
                    dest[i * 4 + 1] = src[i * 3 + 1];
                    dest[i * 4 + 2] = src[i * 3 + 2];
                    dest[i * 4 + 3] = 255;
                }
            }
            else
            {
                switch (_config.channels)
                {
                    case 1:
                        _config.format = Format::R8_UNORM;
                        break;
                    case 2:
                        _config.format = Format::RG8_UNORM;
                        break;
                    case 4:
                        _config.format = Format::RGBA8_UNORM;
                        break;
                    default:
                        _config.format = Format::RGBA8_UNORM;
                        break;
                }

                size_t srcSize = width * height * _config.channels;
                _cpuData.resize(srcSize);
                std::memcpy(_cpuData.data(), data, srcSize);
            }
        }

        if (shouldFreeSTB && data)
        {
            SetStatus(AssetStatus::Loading);
            stbi_image_free(data);
        }
    }

    bool TextureDX11::SaveToFile(const std::string& path) const
    {
        // Get data
        std::vector<uint8_t> rawData = GetData();

        if (rawData.empty())
        {
            FT_ENGINE_ERROR("TextureDX11::SaveToFile failed: No data retrieved from texture.");
            return false;
        }

        int components = 4;

        switch (_config.format)
        {
            case Format::R8_UNORM:
                components = 1;
                break;
            case Format::RG8_UNORM:
                components = 2;
                break;
            case Format::RGBA8_UNORM:
                components = 4;
                break;
            default:
                if (_config.format != Format::RGBA8_UNORM)
                {
                    FT_ENGINE_WARN("TextureDX11::SaveToFile: Potential format mismatch. Saving as 4 components.");
                }
                components = 4;
                break;
        }
        try
        {
            std::filesystem::path fsPath(path);
            if (fsPath.has_parent_path() && !std::filesystem::exists(fsPath.parent_path()))
            {
                std::filesystem::create_directories(fsPath.parent_path());
            }
        }
        catch (const std::exception& e)
        {
            FT_ENGINE_ERROR("TextureDX11::SaveToFile: FS Error: {}", e.what());
            return false;
        }

        stbi_flip_vertically_on_write(0);

        int result = stbi_write_png(
            path.c_str(), _config.width, _config.height, components, rawData.data(), _config.width * components);

        if (result == 0)
        {
            FT_ENGINE_ERROR("TextureDX11::SaveToFile failed to write to {}", path);
            return false;
        }

        return true;
    }

    void TextureDX11::_CreateCubemapCPU()
    {
        _config.channels = 4;
        _config.format = Format::RGBA8_UNORM;
        _cpuCubemapData.clear();
        _cpuCubemapData.resize(6);

        if (_config.isUnfoldedCubemap)
        {
            int width = 0, height = 0, channels = 0;
            stbi_uc* unfoldedData = nullptr;
            bool success = false;

            _mmFile = std::make_unique<MemoryMappedFile>(_config.path);
            if (_mmFile->IsValid())
            {
                unfoldedData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(_mmFile->GetData()),
                                                     static_cast<int>(_mmFile->GetSize()),
                                                     &width,
                                                     &height,
                                                     &channels,
                                                     4);
            }

            if (!unfoldedData)
            {
                FT_ENGINE_ERROR("Failed to load unfolded cubemap texture: {0}", _config.path);
                SetStatus(AssetStatus::Failed);
                return;
            }

            int faceW = 0, faceH = 0;

            // Strip
            if (width % 6 == 0 && width / 6 == height)
            {
                faceW = width / 6;
                faceH = height;

                for (int i = 0; i < 6; ++i)
                {
                    size_t dataSize = (size_t)faceW * faceH * 4;
                    _cpuCubemapData[i].resize(dataSize);

                    for (int y = 0; y < faceH; ++y)
                    {
                        const stbi_uc* src = unfoldedData + (y * width + (i * faceW)) * 4;
                        uint8_t* dst = _cpuCubemapData[i].data() + (y * faceW) * 4;
                        std::memcpy(dst, src, faceW * 4);
                    }
                }
                success = true;
            }
            // Cross (4x3 or 3x4)
            else if ((width % 4 == 0 && width / 4 == height / 3) || (width % 3 == 0 && width / 3 == height / 4))
            {
                faceW = width / 4;
                faceH = height / 3;
                if (width % 3 == 0 && width / 3 == height / 4)
                {
                    faceW = width / 3;
                    faceH = height / 4;
                }

                for (int i = 0; i < 6; ++i)
                {
                    int srcX = CROSS_FACE_COORDS[i].x * faceW;
                    int srcY = CROSS_FACE_COORDS[i].y * faceH;

                    if (srcX + faceW > width || srcY + faceH > height)
                    {
                        FT_ENGINE_ERROR("Cubemap face coords out of bounds.");
                        break;
                    }

                    size_t dataSize = (size_t)faceW * faceH * 4;
                    _cpuCubemapData[i].resize(dataSize);

                    for (int y = 0; y < faceH; ++y)
                    {
                        const stbi_uc* src = unfoldedData + ((srcY + y) * width + srcX) * 4;
                        uint8_t* dst = _cpuCubemapData[i].data() + (y * faceW) * 4;
                        std::memcpy(dst, src, faceW * 4);
                    }
                }
                success = true;
            }
            else
            {
                FT_ENGINE_ERROR("Unfolded cubemap aspect ratio unknown: {}x{}", width, height);
            }

            stbi_image_free(unfoldedData);

            if (success)
            {
                _config.width = faceW;
                _config.height = faceH;
            }
            else
            {
                SetStatus(AssetStatus::Failed);
            }
        }
        else if (_config.faceFilePaths.size() == 6)
        {
            int w = 0, h = 0, c = 0;
            bool fail = false;

            for (int i = 0; i < 6; ++i)
            {
                stbi_uc* data = stbi_load(_config.faceFilePaths[i].c_str(), &w, &h, &c, 4);

                if (!data)
                {
                    FT_ENGINE_ERROR("Failed to load cubemap face: {}", _config.faceFilePaths[i]);
                    fail = true;
                    break;
                }

                if (i == 0)
                {
                    _config.width = w;
                    _config.height = h;
                }
                else if (w != _config.width || h != _config.height)
                {
                    FT_ENGINE_ERROR("Cubemap faces must have same dimensions.");
                    stbi_image_free(data);
                    fail = true;
                    break;
                }

                size_t size = w * h * 4;
                _cpuCubemapData[i].resize(size);
                std::memcpy(_cpuCubemapData[i].data(), data, size);
                stbi_image_free(data);
            }

            if (fail)
            {
                _cpuCubemapData.clear();
                SetStatus(AssetStatus::Failed);
            }
        }
        else
        {
            FT_ENGINE_ERROR("Invalid Cubemap config provided.");
            SetStatus(AssetStatus::Failed);
        }
    }

    void TextureDX11::UploadGPU()
    {
        if (GetStatus() == AssetStatus::Loaded || GetStatus() == AssetStatus::Failed)
        {
            _ReleaseCPUData();
            return;
        }

        if (_cpuData.empty() && _cpuCubemapData.empty() && !_config.isRenderTarget && !_config.path.empty())
        {
            _ReleaseCPUData();
            SetStatus(AssetStatus::Failed);
            return;
        }

        if (_config.width == 0 || _config.height == 0)
        {
            _ReleaseCPUData();
            SetStatus(AssetStatus::Failed);
            return;
        }

        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = renderer->GetDevice();
        ID3D11DeviceContext* context = renderer->GetDeviceContext();

        if (_config.layout == TextureLayout::CUBEMAP)
        {
            if (_cpuCubemapData.empty() || _cpuCubemapData[0].empty())
            {
                _ReleaseCPUData();
                SetStatus(AssetStatus::Failed);
                return;
            }

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = _config.width;
            texDesc.Height = _config.height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 6;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.Usage = D3D11_USAGE_IMMUTABLE;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

            D3D11_SUBRESOURCE_DATA subRes[6];
            for (int i = 0; i < 6; ++i)
            {
                subRes[i].pSysMem = _cpuCubemapData[i].data();
                subRes[i].SysMemPitch = _config.width * 4; // RGBA
                subRes[i].SysMemSlicePitch = 0;
            }

            HRESULT hr = device->CreateTexture2D(&texDesc, subRes, _texture.GetAddressOf());

            if (FAILED(hr))
            {
                FT_ENGINE_ERROR("D3D11 Create Cubemap Failed");
                SetStatus(AssetStatus::Failed);
                _ReleaseCPUData();
                return;
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MipLevels = 1;
            srvDesc.TextureCube.MostDetailedMip = 0;

            hr = device->CreateShaderResourceView(_texture.Get(), &srvDesc, _srv.GetAddressOf());
            FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create Cubemap SRV");

            _cpuCubemapData.clear();
            _mmFile.reset();
            _ReleaseCPUData();
            SetStatus(AssetStatus::Loaded);
            return;
        }

        DXGI_FORMAT dxgiFormat = ToDXGIFormat(_config.format);
        bool generateMips = _config.hasMipmaps && _config.isShaderResource && !_cpuData.empty();
        bool isDepth = IsDepthFormat(_config.format);

        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = _config.width;
        desc.Height = _config.height;
        desc.MipLevels = generateMips ? 0 : 1;
        desc.ArraySize = 1;
        desc.SampleDesc.Count = 1;

        desc.Format = dxgiFormat;

        if (generateMips)
        {
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
        else
        {
            if (!_cpuData.empty() && !_config.isRenderTarget)
                desc.Usage = D3D11_USAGE_IMMUTABLE;
            else
                desc.Usage = D3D11_USAGE_DEFAULT;

            desc.BindFlags = 0;
            if (_config.isShaderResource)
                desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

            if (_config.isRenderTarget)
            {
                if (isDepth)
                    desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                else
                    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
            }
        }

        DXGI_FORMAT textureFormat = dxgiFormat;
        DXGI_FORMAT srvFormat = dxgiFormat;
        DXGI_FORMAT dsvFormat = dxgiFormat;

        if (isDepth && (_config.isShaderResource))
        {
            if (dxgiFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
            {
                textureFormat = DXGI_FORMAT_R24G8_TYPELESS;
                srvFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                dsvFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
            }
            else if (dxgiFormat == DXGI_FORMAT_D32_FLOAT)
            {
                textureFormat = DXGI_FORMAT_R32_TYPELESS;
                srvFormat = DXGI_FORMAT_R32_FLOAT;
                dsvFormat = DXGI_FORMAT_D32_FLOAT;
            }
            desc.Format = textureFormat;
        }

        HRESULT hr;
        if (generateMips)
        {
            hr = device->CreateTexture2D(&desc, nullptr, _texture.GetAddressOf());
            if (SUCCEEDED(hr) && !_cpuData.empty())
            {
                context->UpdateSubresource(
                    _texture.Get(), 0, nullptr, _cpuData.data(), _config.width * GetFormatSize(_config.format), 0);
            }
        }
        else if (!_cpuData.empty())
        {
            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = _cpuData.data();
            initData.SysMemPitch = _config.width * GetFormatSize(_config.format);
            hr = device->CreateTexture2D(&desc, &initData, _texture.GetAddressOf());
        }
        else
        {
            hr = device->CreateTexture2D(&desc, nullptr, _texture.GetAddressOf());
        }

        if (FAILED(hr))
        {
            FT_ENGINE_ERROR("Failed to create DX11 Texture: {} (Format: {})", _config.debugName, (int)dxgiFormat);
            SetStatus(AssetStatus::Failed);
            _ReleaseCPUData();
            return;
        }

        if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = dxgiFormat == DXGI_FORMAT_R24G8_TYPELESS ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : dxgiFormat;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = generateMips ? -1 : 1;
            srvDesc.Texture2D.MostDetailedMip = 0;

            hr = device->CreateShaderResourceView(_texture.Get(), &srvDesc, _srv.GetAddressOf());
            if (FAILED(hr))
            {
                FT_ENGINE_ERROR("Failed to create SRV for {}", _config.debugName);
                _ReleaseCPUData();
            }
        }

        // Render Target View
        if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
        {
            device->CreateRenderTargetView(_texture.Get(), nullptr, _rtv.GetAddressOf());
        }

        // Depth Stencil View
        if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;

            hr = device->CreateDepthStencilView(_texture.Get(), &dsvDesc, _dsv.GetAddressOf());
            if (FAILED(hr))
            {
                FT_ENGINE_ERROR("Failed to create DSV for {}", _config.debugName);
                _ReleaseCPUData();
            }
        }

        if (generateMips && _srv)
        {
            context->GenerateMips(_srv.Get());
        }

        _ReleaseCPUData();
        SetStatus(AssetStatus::Loaded);
    }

    void TextureDX11::_ReleaseCPUData()
    {
        if (!_cpuData.empty())
        {
            _cpuData.clear();
            _cpuData.shrink_to_fit();
        }

        if (!_cpuCubemapData.empty())
        {
            _cpuCubemapData.clear();
        }

        if (!_config.fileData.empty())
        {
            _config.fileData.clear();
            _config.fileData.shrink_to_fit();
        }

        _mmFile.reset();
    }

    void TextureDX11::Bind(Slot slot) const
    {
        if (!IsLoaded() || !_srv)
            return;
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        renderer->GetDeviceContext()->PSSetShaderResources(static_cast<UINT>(slot), 1, _srv.GetAddressOf());
    }

    const std::vector<uint8_t> TextureDX11::GetData() const
    {
        if (!IsLoaded() || !_texture)
            return {};
        if (_dataCached)
            return _dataCache;

        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11DeviceContext* context = renderer->GetDeviceContext();
        ID3D11Device* device = renderer->GetDevice();

        D3D11_TEXTURE2D_DESC desc;
        _texture->GetDesc(&desc);

        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
        HRESULT hr = device->CreateTexture2D(&desc, nullptr, stagingTexture.GetAddressOf());
        if (FAILED(hr))
            return {};

        context->CopyResource(stagingTexture.Get(), _texture.Get());

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        hr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
        if (FAILED(hr))
            return {};

        const uint32_t bytesPerPixel = GetFormatSize(_config.format);
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

} // namespace Frost