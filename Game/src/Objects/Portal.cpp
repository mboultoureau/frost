#include "Portal.h"
#include "../Game.h"

#include <Jolt/Core/Core.h>

#include "../MainLayer.h"
#include "../Physics/PhysicsLayer.h"
#include "Player/PlayerCamera.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;

PortalScript::PortalScript(GameObject playerId, PortalType type, GameObject& linkedId) :
    playerId(playerId), linkedPortalId(linkedId), portalType(type)
{
}

PortalScript::PortalScript(GameObject playerId, PortalType type) :
    playerId(playerId), portalType(type), linkedPortalId{}
{
}

void
PortalScript::OnInitialize()
{
    using namespace JPH;

    if (!_gameObject.HasComponent<StaticMesh>())
    {
        _gameObject.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/portal4.glb" });
    }
    if (_gameObject.HasComponent<RigidBody>())
    {
        return;
    }

    RigidBody rbComp{ ShapeBox{ Vector3{ 5.0f, 5.0f, 5.0f } }, ObjectLayers::PORTAL, RigidBody::MotionType::Static };
    rbComp.isSensor = true;

    auto& rb = _gameObject.AddComponent<RigidBody>(rbComp);
}

void
PortalScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    if (!linkedPortalId.has_value())
        return;
    if (portalType == PortalType::Exit)
        return;

    WarpPlayer();
}

void
PortalScript::WarpPlayer()
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    auto player = mainLayer->GetPlayer();

    auto playerTransform = scene.GetComponent<WorldTransform>(playerId);
    auto playerRb = scene.GetComponent<RigidBody>(playerId);
    auto playerBodyId = playerRb->runtimeBodyID;
    auto bodyInter = Physics::Get().body_interface;

    auto& playerCamera = player->GetCamera()->GetCameraId();

    Vector3 playerSpeed = Math::vector_cast<Vector3>(bodyInter->GetLinearVelocity(playerBodyId));

    auto portal1Transform = scene.GetComponent<WorldTransform>(GetGameObject());
    auto portal2Transform = scene.GetComponent<WorldTransform>(linkedPortalId.value());
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
}

Portal::Portal(Vector3 position, EulerAngles rotation, Vector3 scale, Player* player) :
    _playerController{ player }, _portal{ Game::GetScene().CreateGameObject("Portal") }
{
    Game::GetScene().AddComponent<Transform>(_portal, position, rotation, scale);
    Game::GetScene().AddComponent<WorldTransform>(_portal);
}

void
Portal::SetupPortal(PortalType type, GameObject& otherPortal)
{
    Game::GetScene().AddScript<PortalScript>(_portal, _playerController->GetPlayerID(), type, otherPortal);
}

void
Portal::SetupPortal(PortalType type)
{
    Game::GetScene().AddScript<PortalScript>(_portal, _playerController->GetPlayerID(), type);
}
