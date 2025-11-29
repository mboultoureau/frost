#include "Frost/Renderer/DX11/FormatDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"

namespace Frost
{
    DXGI_FORMAT ToDXGIFormat(Format format)
    {
        switch (format)
        {
            case Format::D24_UNORM_S8_UINT:
                return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Format::R8_UNORM:
                return DXGI_FORMAT_R8_UNORM;
            case Format::RG8_UNORM:
                return DXGI_FORMAT_R8G8_UNORM;
            case Format::RGBA8_UNORM:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Format::RGBA16_FLOAT:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case Format::RG32_FLOAT:
                return DXGI_FORMAT_R32G32_FLOAT;
            case Format::RGB32_FLOAT:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case Format::RGBA32_FLOAT:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case Format::RGB10A2_UNORM:
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Format::R11G11B10_FLOAT:
                return DXGI_FORMAT_R11G11B10_FLOAT;
            case Format::R16_FLOAT:
                return DXGI_FORMAT_R16_FLOAT;
            case Format::R32_FLOAT:
                return DXGI_FORMAT_R32_FLOAT;
            case Format::R24G8_TYPELESS:
                return DXGI_FORMAT_R24G8_TYPELESS;
            default:
                FT_ENGINE_ASSERT(false, "Unsupported format specified: {}", static_cast<int>(format));
                return DXGI_FORMAT_UNKNOWN;
        }
    }

    uint32_t GetFormatSize(Format format)
    {
        switch (format)
        {
            case Format::D24_UNORM_S8_UINT:
                return 4;
            case Format::R8_UNORM:
                return 1;
            case Format::RG8_UNORM:
                return 2;
            case Format::RGBA8_UNORM:
                return 4;
            case Format::RGBA16_FLOAT:
                return 8;
            case Format::RG32_FLOAT:
                return 8;
            case Format::RGB32_FLOAT:
                return 12;
            case Format::RGBA32_FLOAT:
                return 16;
            case Format::RGB10A2_UNORM:
                return 4;
            case Format::R11G11B10_FLOAT:
                return 4;
            case Format::R16_FLOAT:
                return 2;
            case Format::R32_FLOAT:
                return 4;
            case Format::R24G8_TYPELESS:
                return 4;
            default:
                FT_ENGINE_ASSERT(false, "Unsupported format specified");
                return 0;
        }
    }
} // namespace Frost
