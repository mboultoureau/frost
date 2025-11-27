#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Renderer/Pipeline/SkyboxPipeline.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/VirtualCamera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/ECS/System.h"

#include <entt/entt.hpp>
#include <memory>

namespace Frost
{
    struct RenderCameraData
    {
        entt::entity entity;
        const Component::Camera* camera;
        const Component::WorldTransform* transform;
    };

    class RendererSystem : public System
    {
    public:
        RendererSystem();
        void LateUpdate(Scene& scene, float deltaTime) override;

    private:
        void _RenderSceneForCamera(Scene& scene,
                                   const RenderCameraData& camData,
                                   float deltaTime,
                                   const std::vector<std::pair<Component::Light, Component::WorldTransform>>& allLights,
                                   const std::shared_ptr<Texture>& skyboxTexture,
                                   Texture* overrideRenderTarget = nullptr,
                                   float overrideAspectRatio = 0.0f);

    private:
        DeferredRenderingPipeline _deferredRendering;
        SkyboxPipeline _skyboxPipeline;

        std::unique_ptr<Texture> _source;
        std::unique_ptr<Texture> _destination;
    };
} // namespace Frost
