#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/CommandList.h"
#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Renderer/Pipeline/SkyboxPipeline.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Light.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/VirtualCamera.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/Frustum.h"
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
        void SetRenderTargetOverride(std::shared_ptr<Texture> target) { _externalRenderTarget = target; }

    private:
        void _RenderSceneToTexture(Scene& scene,
                                   const RenderCameraData& camData,
                                   float deltaTime,
                                   const std::vector<std::pair<Component::Light, Component::WorldTransform>>& allLights,
                                   const std::shared_ptr<Texture>& skybox,
                                   float overrideAspectRatio);

        void _ApplyPostProcessing(CommandList* commandList,
                                  Texture* sourceTexture,
                                  Texture* destinationTarget,
                                  const Component::Camera& camera,
                                  float deltaTime);

        bool _IsVisible(const Component::StaticMesh& staticMesh, const Math::Matrix4x4& worldMatrix);

    private:
        DeferredRenderingPipeline _deferredRendering;
        SkyboxPipeline _skyboxPipeline;

        std::shared_ptr<Texture> _source;
        std::shared_ptr<Texture> _destination;

        std::shared_ptr<Texture> _externalRenderTarget = nullptr;
        uint32_t _viewportWidth = 0;
        uint32_t _viewportHeight = 0;

        Frustum _frustum;
    };
} // namespace Frost
