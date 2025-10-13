#pragma once

#include "Frost/Scene/ECS/System.h"

namespace Frost
{
    class WorldTransformSystem : public System
    {
    public:
        WorldTransformSystem();
        void Update(Frost::ECS& ecs, float deltaTime) override;

    private:
        void Render(ECS& ecs);
    };
}
