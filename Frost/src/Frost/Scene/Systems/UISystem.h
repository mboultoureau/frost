#pragma once

#include "Frost/Renderer/Pipeline/HUDRenderingPipeline.h"
#include "Frost/Scene/ECS/System.h"

namespace Frost
{
    class UISystem : public System
    {
    public:
        UISystem();
        void Update(Scene& scene, float deltaTime) override;
        void LateUpdate(Scene& scene, float deltaTime) override;

    private:
        HUDRenderingPipeline _pipeline;
    };
} // namespace Frost
