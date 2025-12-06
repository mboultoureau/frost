/*
// Fichier : Frost/Scene/Systems/BillboardSystem.h

#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Pipeline/BillboardRenderingPipeline.h"

namespace Frost
{
    class Scene;

    class BillboardSystem : public System
    {
    public:
        BillboardSystem();
        ~BillboardSystem();

        void LateUpdate(Scene& scene, float deltaTime) override;

    private:
        BillboardRenderingPipeline _pipeline;
    };
} // namespace Frost*/