#include "Portal.h"
#include "Frost/Scene/Components/RelativeView.h"

#include "../Game.h"
#include <string>

using namespace Frost::Component;
using namespace Frost::Math;

Portal::Portal(Vector3 position, EulerAngles rotation)
{
    auto& scene = Game::GetScene();

    // Portal
    _portalObject = scene.CreateGameObject("Portal");

    // Camera
    _cameraObject = scene.CreateGameObject("Portal Camera", _portalObject);
    auto& camera = _cameraObject.AddComponent<VirtualCamera>();

    TextureConfig textureConfig{
        .format = Format::RGBA8_UNORM,
        .width = 1024,
        .height = 1024,
        .isRenderTarget = true,
    };

    _renderTarget = AssetManager::LoadAsset("PortalRT_" + std::to_string(static_cast<uint32_t>(_cameraObject.GetId())),
                                            textureConfig);

    camera.SetRenderTarget(_renderTarget);

    // Portal frame
    _frameObject = scene.CreateGameObject("Portal Frame", _portalObject);
    auto& mesh = _frameObject.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/portal.fbx" }, true);

    // Initial position and rotation
    auto& transform = _portalObject.GetComponent<Transform>();
    transform.position = position;
    transform.SetRotation(rotation);
    transform.scale = { 0.03, 0.03, 0.03 };
}

void
Portal::LinkTo(Portal* other)
{
    _linkedPortal = other;
    _materialLinkPending = true;

    auto& relativeView = other->_cameraObject.AddComponent<RelativeView>();
    relativeView.referenceEntity = this->_portalObject.GetId();
    relativeView.modifier = Matrix4x4::CreateFromQuaternion(Vector4(0, 1, 0, 0)); // 180 deg Y
}

void
Portal::Update()
{
    if (_materialLinkPending && _linkedPortal)
    {
        auto& mesh = _frameObject.GetComponent<StaticMesh>();
        auto& model = mesh.GetModel();

        if (model)
        {
            auto& materials = model->GetMaterials();
            if (!materials.empty())
            {
                materials[0].cameraRef = _linkedPortal->_cameraObject.GetId();
                _materialLinkPending = false;
            }
        }
    }

    auto* window = Application::GetWindow();
    uint32_t width = window->GetWidth();
    uint32_t height = window->GetHeight();

    if (_renderTarget && (_renderTarget->GetWidth() != width || _renderTarget->GetHeight() != height))
    {
        TextureConfig textureConfig{
            .format = Format::RGBA8_UNORM,
            .width = width,
            .height = height,
            .isRenderTarget = true,
        };

        _renderTarget = Texture::Create(textureConfig);

        auto& camera = _cameraObject.GetComponent<VirtualCamera>();
        camera.SetRenderTarget(_renderTarget);
        camera.useScreenSpaceAspectRatio = true;
    }
}
