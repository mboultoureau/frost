#pragma once

#include "Frost/Renderer/GPUResource.h"

#include <cfloat>
#include <cstdint>

namespace Frost
{
    enum class Filter
    {
        MIN_MAG_MIP_POINT,
        MIN_MAG_POINT_MIP_LINEAR,
        MIN_POINT_MAG_LINEAR_MIP_POINT,
        MIN_POINT_MAG_MIP_LINEAR,
        MIN_LINEAR_MAG_MIP_POINT,
        MIN_LINEAR_MAG_POINT_MIP_LINEAR,
        MIN_MAG_LINEAR_MIP_POINT,
        MIN_MAG_MIP_LINEAR,
        ANISOTROPIC,
    };

    enum class AddressMode
    {
        WRAP,
        MIRROR,
        CLAMP,
        BORDER,
        MIRROR_ONCE
    };

    enum class ComparisonFunction
    {
        NEVER,
        LESS_EQUAL,
    };

    struct SamplerConfig
    {
        Filter filter = Filter::MIN_MAG_MIP_LINEAR;
        AddressMode addressU = AddressMode::WRAP;
        AddressMode addressV = AddressMode::WRAP;
        AddressMode addressW = AddressMode::WRAP;
        ComparisonFunction comparisonFunction = ComparisonFunction::NEVER;
        float mipLODBias = 0.0f;
        uint32_t maxAnisotropy = 1;
        float borderColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float minLOD = -FLT_MAX;
        float maxLOD = FLT_MAX;
    };

    class Sampler : public GPUResource
    {
    public:
        virtual ~Sampler() = default;

        virtual const SamplerConfig& GetConfig() const = 0;
    };
} // namespace Frost