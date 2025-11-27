#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Scene/Scene.h"

namespace Frost
{
    class ScriptableSystem : public System
    {
    public:
        void Update(Scene& scene, float deltaTime) override;
        void PreFixedUpdate(Scene& scene, float deltaTime) override;
        void FixedUpdate(Scene& scene, float fixedDeltaTime) override;
        void LateUpdate(Scene& scene, float deltaTime) override;
    };
} // namespace Frost
