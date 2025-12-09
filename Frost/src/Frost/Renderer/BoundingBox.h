#pragma once

#include "Frost/Core/Core.h"

#include <DirectXMath.h>
#include <algorithm>

#undef min
#undef max

namespace Frost
{
    struct FROST_API BoundingBox
    {
        DirectX::XMFLOAT3 min; // coin inferieur (x-, y-, z-)
        DirectX::XMFLOAT3 max; // coin superieur (x+, y+, z+)

        static BoundingBox TransformAABB(const BoundingBox& box, const DirectX::XMMATRIX& world)
        {
            using namespace DirectX;
            XMFLOAT3 corners[8] = { { box.min.x, box.min.y, box.min.z }, { box.max.x, box.min.y, box.min.z },
                                    { box.min.x, box.max.y, box.min.z }, { box.max.x, box.max.y, box.min.z },
                                    { box.min.x, box.min.y, box.max.z }, { box.max.x, box.min.y, box.max.z },
                                    { box.min.x, box.max.y, box.max.z }, { box.max.x, box.max.y, box.max.z } };

            XMFLOAT3 newMin = { FLT_MAX, FLT_MAX, FLT_MAX };
            XMFLOAT3 newMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            for (int i = 0; i < 8; ++i)
            {
                XMVECTOR v = XMVector3Transform(XMLoadFloat3(&corners[i]), world);
                XMFLOAT3 p;
                XMStoreFloat3(&p, v);

                newMin.x = newMin.x < p.x ? newMin.x : p.x;
                newMin.y = newMin.y < p.y ? newMin.y : p.y;
                newMin.z = newMin.z < p.z ? newMin.z : p.z;

                newMax.x = newMax.x > p.x ? newMax.x : p.x;
                newMax.y = newMax.y > p.y ? newMax.y : p.y;
                newMax.z = newMax.z > p.z ? newMax.z : p.z;
            }

            return { newMin, newMax };
        }

        void Merge(const BoundingBox& other)
        {
            min.x = std::min(min.x, other.min.x);
            min.y = std::min(min.y, other.min.y);
            min.z = std::min(min.z, other.min.z);

            max.x = std::max(max.x, other.max.x);
            max.y = std::max(max.y, other.max.y);
            max.z = std::max(max.z, other.max.z);
        }
    };
} // namespace Frost