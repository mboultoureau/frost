/*#include "Frost/Scene/Systems/BillboardSystem.h"

#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Utils/Math/Matrix.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Billboard.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Utils/Math/Vector.h"

using namespace Frost::Component;

namespace Frost
{
    BillboardSystem::BillboardSystem()
    {
    }

    BillboardSystem::~BillboardSystem()
    {
    }


    void BillboardSystem::LateUpdate(Scene& scene, float deltaTime)
    {
        const Component::WorldTransform* cameraTransform = nullptr;
        const Component::Camera* cameraComp = nullptr;

        auto cameraView = scene.ViewActive<Component::Camera, Component::WorldTransform>();

        cameraView.each(
            [&](auto entity, auto& cam, auto& transform)
            {
                if (cameraComp == nullptr || cam.priority > cameraComp->priority)
                {
                    cameraComp = &cam;
                    cameraTransform = &transform;
                }
            });

        if (cameraComp == nullptr || cameraTransform == nullptr)
        {
            return;
        }

        Texture* backBuffer = RendererAPI::GetRenderer()->GetBackBuffer();
        if (backBuffer == nullptr || backBuffer->GetWidth() == 0 || backBuffer->GetHeight() == 0)
        {
            return;
        }

        float currentWidth = (float)backBuffer->GetWidth();
        float currentHeight = (float)backBuffer->GetHeight();
        float aspectRatio = currentWidth / currentHeight;

        Frost::Math::Matrix4x4 viewMatrix = Frost::Math::GetViewMatrix(*cameraTransform);
        Frost::Math::Matrix4x4 projectionMatrix = Frost::Math::GetProjectionMatrix(*cameraComp, aspectRatio);

        _pipeline.BeginFrame(viewMatrix,
                             projectionMatrix,
                             RendererAPI::GetRenderer()->GetBackBuffer(),
                             RendererAPI::GetRenderer()->GetDepthBuffer());

        auto billboardView = scene.ViewActive<Component::Billboard, Component::WorldTransform>();

        billboardView.each(
            [&](auto entity, auto& billboard, auto& transform)
            {

                BillboardConstants constants;

                constants.Position = transform.position;
                constants.Width = billboard.GetWidth();
                constants.Height = billboard.GetHeight();

                constants.CenteredY = billboard.centeredY ? 1.0f : 0.0f;

                if (billboard.texture)
                {
                    _pipeline.Submit(constants, billboard.texture, billboard.textureFilter);
                }
            });

        _pipeline.EndFrame();
    }
} // namespace Frost*/