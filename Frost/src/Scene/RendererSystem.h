#pragma once

#include "Scene/ECS/System.h"

namespace Frost
{
    class RendererSystem : public ISystem
    {
    public:
        void Update(Frost::ECS& ecs, float dt) override
        {
        }

        Frost::SystemType GetSystemType() const override
        {
            return Frost::SystemType::Update;
        }
    };
}