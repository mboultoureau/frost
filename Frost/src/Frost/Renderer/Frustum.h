#pragma once
#include "Frost/Renderer/BoundingBox.h"
#include <DirectXMath.h>

namespace Frost
{
    struct Plane
    {
        DirectX::XMFLOAT4 equation; // a, b, c, d
    };

    class Frustum
    {
    public:
        Plane planes[6];

        void Extract(const DirectX::XMMATRIX& viewProj, float margin);
        bool IsInside(const BoundingBox& box) const;
    };
} // namespace Frost