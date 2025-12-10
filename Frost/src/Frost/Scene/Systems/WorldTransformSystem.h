#pragma once

#include "Frost/Scene/ECS/System.h"

#include <DirectXMath.h>

namespace Frost
{
    class WorldTransformSystem : public System
    {
    public:
        WorldTransformSystem();
        void PreFixedUpdate(Scene& scene, float deltaTime) override;

    private:
        void _UpdateHierarchy(entt::registry& registry,
                              entt::entity entity,
                              DirectX::XMVECTOR parentPosition,
                              DirectX::XMVECTOR parentRotation,
                              DirectX::XMVECTOR parentScale);
    };
} // namespace Frost
