#include "Frustum.h"

using namespace DirectX;

namespace Frost
{
    void Frustum::Extract(const DirectX::XMMATRIX& viewProj, float margin)
    {
        using namespace DirectX;

        XMFLOAT4X4 m;
        XMStoreFloat4x4(&m, viewProj);

        // Gauche
        planes[0].equation = { m._14 + m._11, m._24 + m._21, m._34 + m._31, m._44 + m._41 };
        // Droite
        planes[1].equation = { m._14 - m._11, m._24 - m._21, m._34 - m._31, m._44 - m._41 };
        // Bas
        planes[2].equation = { m._14 + m._12, m._24 + m._22, m._34 + m._32, m._44 + m._42 };
        // Haut
        planes[3].equation = { m._14 - m._12, m._24 - m._22, m._34 - m._32, m._44 - m._42 };
        // Near
        planes[4].equation = { m._13, m._23, m._33, m._43 };
        // Far
        planes[5].equation = { m._14 - m._13, m._24 - m._23, m._34 - m._33, m._44 - m._43 };

        // Normalisation des plans + application du padding
        for (int i = 0; i < 6; ++i)
        {
            XMVECTOR p = XMLoadFloat4(&planes[i].equation);

            XMVECTOR n = XMVectorSet(XMVectorGetX(p), XMVectorGetY(p), XMVectorGetZ(p), 0.f);
            float d = XMVectorGetW(p);

            // Normalisation
            float len = XMVectorGetX(XMVector3Length(n));
            n = XMVectorScale(n, 1.f / len);
            d /= len;

            // Application de la marge
            d += margin;

            XMStoreFloat4(&planes[i].equation, XMVectorSet(XMVectorGetX(n), XMVectorGetY(n), XMVectorGetZ(n), d));
        }
    }

    bool Frustum::IsInside(const BoundingBox& box) const
    {

        // 8 coins du cube
        XMFLOAT3 corners[8] = { { box.min.x, box.min.y, box.min.z }, { box.max.x, box.min.y, box.min.z },
                                { box.min.x, box.max.y, box.min.z }, { box.max.x, box.max.y, box.min.z },
                                { box.min.x, box.min.y, box.max.z }, { box.max.x, box.min.y, box.max.z },
                                { box.min.x, box.max.y, box.max.z }, { box.max.x, box.max.y, box.max.z } };

        // Pour chaque plan du frustum
        for (int i = 0; i < 6; ++i)
        {
            const auto& eq = planes[i].equation;

            int insideCount = 0;
            for (int j = 0; j < 8; ++j)
            {
                float dist = eq.x * corners[j].x + eq.y * corners[j].y + eq.z * corners[j].z + eq.w;
                if (dist >= 0.0f)
                {
                    insideCount++;
                    break; // un coin suffit a prouver que la boite n est pas
                           // entierement derriere le plan
                }
            }

            if (insideCount == 0)
                return false; // completement en dehors du frustum
        }

        return true; // au moins partiellement visible
    }
} // namespace Frost
