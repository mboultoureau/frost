#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Pipeline/HUDRenderingPipeline.h"
#include "Frost/Renderer/Pipeline/HUDTextRenderingPipeline.h"

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
        HUDTextRenderingPipeline _pipelineText;
    };
} // namespace Frost
