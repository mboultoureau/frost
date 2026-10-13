#pragma once

#include "Frost/Scene/ECS/System.h"

#include <DirectXMath.h>

namespace Frost
{
    class FROST_API WorldTransformSystem : public System
    {
    public:
        WorldTransformSystem();
        void PreFixedUpdate(Scene& scene, float deltaTime) override;
        void Update(Scene& scene, float deltaTime) override;
        void LateUpdate(Scene& scene, float deltaTime) override;

        void SynchronizeWorldTransforms(Scene& scene);

    private:
        void _UpdateHierarchy(entt::registry& registry,
                              entt::entity entity,
                              DirectX::XMVECTOR parentPosition,
                              DirectX::XMVECTOR parentRotation,
                              DirectX::XMVECTOR parentScale,
                              bool parentChanged);
    };
} // namespace Frost
