#include "Portal.h"
#include "../Game.h"

#include <Jolt/Core/Core.h>

#include "../MainLayer.h"
#include "../Physics/PhysicsLayer.h"
#include "Player/PlayerCamera.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Frost/Scene/Components/RelativeView.h>

using namespace Frost;

PortalScript::PortalScript(PortalType type, Portal* linkedPortal, Portal* parent) :
    linkedPortal(linkedPortal), portalType(type), parentPortal(parent)
{
    _materialLinkPending = true;

    auto& relativeView = linkedPortal->_cameraObject.AddComponent<RelativeView>();
    relativeView.referenceEntity = GetGameObject().GetId();
    relativeView.modifier = Matrix4x4::CreateFromQuaternion(Vector4(0, 1, 0, 0)); // 180 deg Y
}

void
PortalScript::OnInitialize()
{
    using namespace JPH;

    if (_gameObject.HasComponent<RigidBody>())
    {
        return;
    }

    RigidBody rbComp{ ShapeBox{ Vector3{ 5.0f, 5.0f, 5.0f } }, ObjectLayers::PORTAL, RigidBody::MotionType::Static };
    rbComp.isSensor = true;

    auto& rb = _gameObject.AddComponent<RigidBody>(rbComp);
}

float
PortalScript::CalculatePortalFOV(const Math::Vector3& cameraPos,
                                 const Math::Vector3& portalPos,
                                 const Math::Vector3& portalScale,
                                 bool isVertical) // true pour FOV vertical, false pour horizontal
{
    // Distance de la camera au portail
    float distance = Math::Length(cameraPos - portalPos);

    // Taille du portail (approximative, ajustez selon votre mesh)
    // Supposons que le portail fait 2 unites de base * scale
    float portalSize = 1000.0f * (isVertical ? portalScale.y : portalScale.x);

    // Calculer l'angle
    float angle = 2.0f * std::atan(portalSize / (2.0f * distance));

    return angle; // En radians
}

void
PortalScript::OnUpdate(float deltaTime)
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();
    auto playerCamera = player->GetCamera();
    auto playerCameraTransform = scene.GetComponent<WorldTransform>(playerCamera->GetCameraId());
    auto portal1Transform = scene.GetComponent<WorldTransform>(GetGameObject());
    // Calculer le FOV base sur la distance
    float verticalFOV =
        CalculatePortalFOV(playerCameraTransform->position, portal1Transform->position, portal1Transform->scale, true);

    // Appliquer a la camera virtuelle
    auto virtualCam = scene.GetComponent<VirtualCamera>(parentPortal->_cameraObject);
    // virtualCam->perspectiveFOV = verticalFOV;

    UpdateChromaticAberrationEffect();

    if (_materialLinkPending && linkedPortal)
    {
        UpdateLinkedCameraRef();
    }

    UpdateCameraRenderTarget();
    UpdateCameraPos();
}

void
PortalScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    if (!linkedPortal)
        return;
    if (portalType == PortalType::Exit)
        return;
    if (portalType == PortalType::Inactive)
        return;

    WarpPlayer();
}

void
PortalScript::WarpPlayer()
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();

    auto playerTransform = scene.GetComponent<WorldTransform>(player->GetPlayerID());
    auto playerRb = scene.GetComponent<RigidBody>(player->GetPlayerID());
    auto playerBodyId = playerRb->runtimeBodyID;
    auto bodyInter = Physics::Get().body_interface;

    auto& playerCamera = player->GetCamera()->GetCameraId();

    Vector3 playerSpeed = Math::vector_cast<Vector3>(bodyInter->GetLinearVelocity(playerBodyId));

    auto portal1Transform = scene.GetComponent<WorldTransform>(GetGameObject());
    auto portal2Transform = scene.GetComponent<WorldTransform>(linkedPortal->_portal);
    auto cameraTransform = scene.GetComponent<WorldTransform>(playerCamera);

    auto cameraRb = scene.GetComponent<RigidBody>(playerCamera);

    Vector3 cameraSpeed = Math::vector_cast<Vector3>(bodyInter->GetLinearVelocity(cameraRb->runtimeBodyID));

    JPH::Quat portal1Rotation{ portal1Transform->rotation.x,
                               portal1Transform->rotation.y,
                               portal1Transform->rotation.z,
                               portal1Transform->rotation.w };
    JPH::Quat portal2Rotation{ portal2Transform->rotation.x,
                               portal2Transform->rotation.y,
                               portal2Transform->rotation.z,
                               portal2Transform->rotation.w };
    JPH::Quat playerRotation{ playerTransform->rotation.x,
                              playerTransform->rotation.y,
                              playerTransform->rotation.z,
                              playerTransform->rotation.w };
    JPH::Quat cameraRotation{ cameraTransform->rotation.x,
                              cameraTransform->rotation.y,
                              cameraTransform->rotation.z,
                              cameraTransform->rotation.w };

    // JPH::Quat rotY180 = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), JPH::DegreesToRadians(180.0f));

    // JPH::Quat portal2Rotation = rotY180*portal2PreEditRotation;

    // Inverse rotation of portal 1
    JPH::Quat rotAInv = portal1Rotation.Conjugated();

    // --- POSITION ---
    JPH::Vec3 localPos = rotAInv * (Math::vector_cast<JPH::Vec3>(playerTransform->position) -
                                    Math::vector_cast<JPH::Vec3>(portal1Transform->position));
    JPH::Vec3 newPlayerPos = Math::vector_cast<JPH::Vec3>(portal2Transform->position) + portal2Rotation * localPos;

    // --- ROTATION ---
    JPH::Quat localRot = rotAInv * playerRotation;
    JPH::Vec4 newPlayerRot = (portal2Rotation * localRot).GetXYZW();

    // --- VELOCITY ---
    JPH::Vec3 localVel = rotAInv * Math::vector_cast<JPH::Vec3>(playerSpeed);
    JPH::Vec3 newPlayerSpeed = portal2Rotation * localVel;

    // --- OFFSET POSITION ---
    JPH::Vec3 offsetLocal = rotAInv * (Math::vector_cast<JPH::Vec3>(cameraTransform->position) -
                                       Math::vector_cast<JPH::Vec3>(playerTransform->position));
    JPH::Vec3 offsetWorld = portal2Rotation * offsetLocal;

    // --- ROTATION ---
    JPH::Quat camLocalRot = rotAInv * cameraRotation;
    JPH::Quat newCamRot = portal2Rotation * camLocalRot;

    // --- VELOCITY ---
    JPH::Vec3 camLocalVel = rotAInv * Math::vector_cast<JPH::Vec3>(cameraSpeed);
    JPH::Vec3 newCamVel = portal2Rotation * camLocalVel;

    player->Warp(Math::vector_cast<Vector3>(newPlayerPos),
                 { newPlayerRot.GetX(), newPlayerRot.GetY(), newPlayerRot.GetZ(), newPlayerRot.GetW() },
                 Math::vector_cast<Vector3>(newPlayerSpeed));

    player->WarpCamera(Math::vector_cast<Vector3>(offsetWorld),
                       { newCamRot.GetX(), newCamRot.GetY(), newCamRot.GetZ(), newCamRot.GetW() },
                       Math::vector_cast<Vector3>(newCamVel));

    _chromaticAberrationTimer.Start();
    _inChromaticEffect = true;
}

void
PortalScript::UpdateChromaticAberrationEffect()
{
    if (!_inChromaticEffect)
        return;

    auto total = std::chrono::duration<float>(_chromaticAberrationDuration).count();
    auto elapsed =
        std::chrono::duration<float>(_chromaticAberrationTimer.GetDurationAs<std::chrono::milliseconds>()).count();

    float chromaticStrength = (total - elapsed) / total;

    if (chromaticStrength < 0)
    {
        _inChromaticEffect = true;
        return;
    }

    chromaticStrength *= _chromaticAberrationFactor;

    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();

    auto playerCamera = player->GetCamera();

    playerCamera->SetChromaticAberrationStrength(chromaticStrength);
}

void
PortalScript::UpdateLinkedCameraRef()
{
    auto& mesh = parentPortal->_frameObject.GetComponent<StaticMesh>();
    auto& model = mesh.GetModel();

    if (model)
    {
        auto& materials = model->GetMaterials();
        if (!materials.empty())
        {
            materials[0].cameraRef = linkedPortal->_cameraObject.GetId();
            _materialLinkPending = false;
        }
    }
}

void
PortalScript::UpdateCameraRenderTarget()
{
    auto* window = Application::GetWindow();
    uint32_t width = window->GetWidth();
    uint32_t height = window->GetHeight();

    if (parentPortal->_renderTarget &&
        (parentPortal->_renderTarget->GetWidth() != width || parentPortal->_renderTarget->GetHeight() != height))
    {
        TextureConfig textureConfig{
            .format = Format::RGBA8_UNORM,
            .width = width,
            .height = height,
            .isRenderTarget = true,
        };

        parentPortal->_renderTarget = Texture::Create(textureConfig);

        auto& camera = parentPortal->_cameraObject.GetComponent<VirtualCamera>();
        auto& screenTexture = parentPortal->_frameObject.AddComponent<ScreenProjectedTexture>();
        camera.SetRenderTarget(parentPortal->_renderTarget);
        screenTexture.SetRenderTarget(parentPortal->_renderTarget);
        camera.useScreenSpaceAspectRatio = true;
    }
}

void
PortalScript::UpdateCameraPos()
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();

    auto playerTransform = scene.GetComponent<WorldTransform>(player->GetPlayerID());

    auto& playerCamera = player->GetCamera()->GetCameraId();

    auto portal1Transform = scene.GetComponent<WorldTransform>(GetGameObject());
    auto portal2Transform = scene.GetComponent<WorldTransform>(linkedPortal->_portal);
    auto cameraTransform = scene.GetComponent<WorldTransform>(playerCamera);

    JPH::Quat portal1Rotation{ portal1Transform->rotation.x,
                               portal1Transform->rotation.y,
                               portal1Transform->rotation.z,
                               portal1Transform->rotation.w };
    JPH::Quat portal2Rotation{ portal2Transform->rotation.x,
                               portal2Transform->rotation.y,
                               portal2Transform->rotation.z,
                               portal2Transform->rotation.w };
    JPH::Quat playerRotation{ playerTransform->rotation.x,
                              playerTransform->rotation.y,
                              playerTransform->rotation.z,
                              playerTransform->rotation.w };
    JPH::Quat cameraRotation{ cameraTransform->rotation.x,
                              cameraTransform->rotation.y,
                              cameraTransform->rotation.z,
                              cameraTransform->rotation.w };

    // Inverse rotation of portal 1
    JPH::Quat rotAInv = portal1Rotation.Conjugated();

    // --- POSITION ---
    JPH::Vec3 localPos = rotAInv * (Math::vector_cast<JPH::Vec3>(playerTransform->position) -
                                    Math::vector_cast<JPH::Vec3>(portal1Transform->position));
    JPH::Vec3 newPlayerPos = Math::vector_cast<JPH::Vec3>(portal2Transform->position) + portal2Rotation * localPos;

    // --- ROTATION ---
    JPH::Quat localRot = rotAInv * playerRotation;
    JPH::Vec4 newPlayerRot = (portal2Rotation * localRot).GetXYZW();

    // --- OFFSET POSITION ---
    JPH::Vec3 offsetLocal = rotAInv * (Math::vector_cast<JPH::Vec3>(cameraTransform->position) -
                                       Math::vector_cast<JPH::Vec3>(playerTransform->position));
    JPH::Vec3 offsetWorld = portal2Rotation * offsetLocal;

    // --- ROTATION ---
    JPH::Quat camLocalRot = rotAInv * cameraRotation;
    JPH::Quat newCamRot = portal2Rotation * camLocalRot;

    auto portalCamTransform = scene.GetComponent<Transform>(linkedPortal->_cameraObject);

    Vector3 newPortalCameraPos = Math::vector_cast<Vector3>(newPlayerPos) + Math::vector_cast<Vector3>(offsetWorld);

    portalCamTransform->position = newPortalCameraPos;
    portalCamTransform->rotation = { newCamRot.GetX(), newCamRot.GetY(), newCamRot.GetZ(), newCamRot.GetW() };
}

Portal::Portal(Vector3 position, EulerAngles rotation, Vector3 scale, Player* player) :
    _portal{ Game::GetScene().CreateGameObject("Portal") }
{
    Game::GetScene().AddComponent<Transform>(_portal, position, rotation, scale);
    Game::GetScene().AddComponent<WorldTransform>(_portal);

    _cameraObject = Game::GetScene().CreateGameObject("Portal Camera");
    auto& camera = _cameraObject.AddComponent<VirtualCamera>();

    TextureConfig textureConfig{
        .format = Format::RGBA8_UNORM,
        .width = 1024,
        .height = 1024,
        .isRenderTarget = true,
    };

    _renderTarget = AssetManager::LoadAsset("PortalRT_" + std::to_string(static_cast<uint32_t>(_cameraObject.GetId())),
                                            textureConfig);

    // Portal frame
    _frameObject = Game::GetScene().CreateGameObject("Portal Frame", _portal);
    auto& screenTexture = _frameObject.AddComponent<ScreenProjectedTexture>();
    // camera.SetRenderTarget(_renderTarget);
    auto& mesh = _frameObject.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/portal.fbx" }, true);

    mesh.hiddenFromCameras.push_back(&camera);
    mesh.overrideFrustumCulling = true;
    camera.portalEntity = _frameObject.GetId();
    camera.postEffects.push_back(std::make_shared<FogEffect>());
    camera.GetEffect<FogEffect>()->SetFog(skyFogMinDepth, skyFogStrength, skyFogColor);
    // std::cout << "Portal mesh triangle count: " << (mesh.GetModel()->GetMeshes()[0].GetIndexCount() / 3) <<
    // std::endl;
}

void
Portal::SetupPortal(PortalType type, Portal* otherPortal)
{
    if (type == PortalType::Exit)
        _frameObject.SetActive(false);
    auto camera = Game::GetScene().GetComponent<VirtualCamera>(_cameraObject);
    camera->linkedPortalEntity = otherPortal->_frameObject.GetId();
    Game::GetScene().AddScript<PortalScript>(_portal, type, otherPortal, this);
}
