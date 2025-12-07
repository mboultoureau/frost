#pragma once

#include "Frost/Renderer/PostEffect.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Vector.h"

#include <memory>
#include <vector>
#include <optional>

namespace Frost
{
    class Texture;
}

namespace Frost::Component
{
    struct RenderTargetConfig
    {
        uint32_t width = 256;
        uint32_t height = 256;

        // Useful for effects like mirror or portal
        bool useScreenSpaceAspectRatio = false;
    };

    struct Camera
    {
        enum class ProjectionType
        {
            Perspective = 0,
            Orthographic = 1
        };

        // Projection
        ProjectionType projectionType = ProjectionType::Perspective;
        Math::Angle<Math::Radian> perspectiveFOV = Math::Angle<Math::Degree>(60.0f);
        float orthographicSize = 10.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;

        // Rendering
        int priority = 0;
        bool frustumCulling = true;
        float frustumPadding = 1.0f;

        bool clearOnRender = true;
        Math::Color4 backgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };

        Viewport viewport = { 0.0f, 0.0f, 1.0f, 1.0f };
        std::optional<RenderTargetConfig> renderTargetConfig;

        // Behavior
        bool lookAtPositionEnabled = false;
        Math::Vector3 lookAtPosition = { 0.0f, 0.0f, 0.0f };

        // Post-Processing
        std::vector<std::shared_ptr<PostEffect>> postEffects;

        template<typename T>
        std::shared_ptr<T> GetEffect()
        {
            for (const auto& effect : postEffects)
            {
                auto castedEffect = std::dynamic_pointer_cast<T>(effect);
                if (castedEffect)
                {
                    return castedEffect;
                }
            }
            return nullptr;
        }
    };
} // namespace Frost::Component