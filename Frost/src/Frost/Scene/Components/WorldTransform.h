#pragma once

#include "Frost/Scene/Components/Transform.h"

#include <limits>

namespace Frost
{
    class WorldTransformSystem;
}

namespace Frost::Component
{
    struct FROST_API WorldTransform : public Transform
    {
        WorldTransform() noexcept : Transform() {}

        WorldTransform(const Math::Vector3& pos, const Math::Vector4& rot, const Math::Vector3& scl) noexcept :
            Transform(pos, rot, scl)
        {
        }

        WorldTransform(const Math::Vector3& pos) noexcept : Transform(pos) {}

    private:
        friend class Frost::WorldTransformSystem;
        Math::Vector3 _lastLocalPosition{ std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f };
        Math::Vector4 _lastLocalRotation{ 0.0f, 0.0f, 0.0f, 0.0f };
        Math::Vector3 _lastLocalScale{ 0.0f, 0.0f, 0.0f };
    };
} // namespace Frost::Component