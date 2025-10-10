#pragma once

#include "Frost/Scene/ECS/System.h"

namespace Frost
{
    class RendererSystem : public System
    {
    public:
        void Update(Frost::ECS& ecs, float deltaTime) override;

    private:
        void Render(ECS& ecs);
    };
}