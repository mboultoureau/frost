#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include <sstream>

namespace Frost
{
    D3D11_FILTER
    ToD3D11Filter(Filter filter)
    {
        switch (filter)
        {
            case Filter::MIN_MAG_MIP_POINT:
                return D3D11_FILTER_MIN_MAG_MIP_POINT;
            case Filter::MIN_MAG_POINT_MIP_LINEAR:
                return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            case Filter::MIN_POINT_MAG_LINEAR_MIP_POINT:
                return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            case Filter::MIN_POINT_MAG_MIP_LINEAR:
                return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            case Filter::MIN_LINEAR_MAG_MIP_POINT:
                return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            case Filter::MIN_LINEAR_MAG_POINT_MIP_LINEAR:
                return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            case Filter::MIN_MAG_LINEAR_MIP_POINT:
                return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            case Filter::MIN_MAG_MIP_LINEAR:
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            case Filter::ANISOTROPIC:
                return D3D11_FILTER_ANISOTROPIC;
        }

        FT_ENGINE_ASSERT(false, "Unsupported filter type specified.");
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }

    D3D11_TEXTURE_ADDRESS_MODE
    ToD3D11AddressMode(AddressMode mode)
    {
        switch (mode)
        {
            case AddressMode::WRAP:
                return D3D11_TEXTURE_ADDRESS_WRAP;
            case AddressMode::MIRROR:
                return D3D11_TEXTURE_ADDRESS_MIRROR;
            case AddressMode::CLAMP:
                return D3D11_TEXTURE_ADDRESS_CLAMP;
            case AddressMode::BORDER:
                return D3D11_TEXTURE_ADDRESS_BORDER;
            case AddressMode::MIRROR_ONCE:
                return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        }
        FT_ENGINE_ASSERT(false, "Unsupported address mode specified.");
        return D3D11_TEXTURE_ADDRESS_WRAP;
    }

    D3D11_COMPARISON_FUNC
    ToD3D11ComparisonFunction(ComparisonFunction comparisonFunction)
    {
        switch (comparisonFunction)
        {
            case ComparisonFunction::NEVER:
                return D3D11_COMPARISON_NEVER;
            case ComparisonFunction::LESS_EQUAL:
                return D3D11_COMPARISON_LESS_EQUAL;
        }
        FT_ENGINE_ASSERT(false, "Unsupported address mode specified.");
        return D3D11_COMPARISON_NEVER;
    }

    SamplerDX11::SamplerDX11(const SamplerConfig& config) : _config(config)
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = renderer->GetDevice();
        FT_ENGINE_ASSERT(device, "D3D11 Device is null!");

        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = ToD3D11Filter(_config.filter);
        desc.AddressU = ToD3D11AddressMode(_config.addressU);
        desc.AddressV = ToD3D11AddressMode(_config.addressV);
        desc.AddressW = ToD3D11AddressMode(_config.addressW);
        desc.MipLODBias = _config.mipLODBias;
        desc.MaxAnisotropy = _config.maxAnisotropy;
        desc.ComparisonFunc = ToD3D11ComparisonFunction(_config.comparisonFunction);
        desc.BorderColor[0] = _config.borderColor[0];
        desc.BorderColor[1] = _config.borderColor[1];
        desc.BorderColor[2] = _config.borderColor[2];
        desc.BorderColor[3] = _config.borderColor[3];
        desc.MinLOD = _config.minLOD;
        desc.MaxLOD = _config.maxLOD;

        HRESULT hr = device->CreateSamplerState(&desc, &_samplerState);

        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 Sampler State.");
    }

    const SamplerConfig& SamplerDX11::GetConfig() const
    {
        return _config;
    }

    ID3D11SamplerState* SamplerDX11::GetSamplerState() const
    {
        return _samplerState.Get();
    }
} // namespace Frost