#pragma once

#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Vector.h"

#include <variant>

namespace Frost::Component
{
    struct LightDirectional
    {
        float cascadeNear = 0;
        float cascadeFar = 100;
        float range = 10.f;
    };

    struct LightPoint
    {
        float radius = 10.0f;
        float falloff = 1.0f;
    };

    struct LightSpot
    {
        float range = 20.0f;
        Math::Angle<Math::Radian> innerConeAngle = Math::Angle<Math::Degree>(20.0f);
        Math::Angle<Math::Radian> outerConeAngle = Math::Angle<Math::Degree>(30.0f);
    };

    struct LightAmbiant
    {
    };

    using LightConfig = std::variant<LightDirectional, LightPoint, LightSpot, LightAmbiant>;

    enum class LightType
    {
        Directional = 0,
        Point = 1,
        Spot = 2,
        Ambiant = 3
    };

    struct Light
    {
        Math::Color3 color = { 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;

        LightConfig config = LightPoint{};

        Light() = default;
        Light(const LightConfig& cfg) : config(cfg) {}

        LightType GetType() const { return (LightType)config.index(); }

        void SetType(LightType type)
        {
            if (GetType() == type)
            {
                return;
            }

            switch (type)
            {
                case LightType::Directional:
                {
                    config = LightDirectional{};
                    break;
                }
                case LightType::Point:
                {
                    config = LightPoint{};
                    break;
                }
                case LightType::Spot:
                {
                    config = LightSpot{};
                    break;
                }
            }
        }
    };
} // namespace Frost::Component