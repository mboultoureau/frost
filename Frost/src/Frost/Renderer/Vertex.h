#pragma once

#include "Frost/Utils/Math/Vector.h"

namespace Frost
{
    struct Vertex
    {
        Math::Vector3 position;
        Math::Vector3 normal;
        Math::Vector2 texCoord;
        Math::Vector4 tangent;
    };
} // namespace Frost