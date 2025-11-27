#pragma once

#include "Frost/Utils/Math/Matrix.h"
#include <entt/entt.hpp>

namespace Frost::Component
{
    struct RelativeView
    {
        entt::entity referenceEntity;
        Math::Matrix4x4 modifier = Math::Matrix4x4::CreateIdentity();
    };
} // namespace Frost::Component
