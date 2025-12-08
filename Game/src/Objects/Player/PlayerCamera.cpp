#include "PlayerCamera.h"

#include "../../Game.h"
#include "../../Physics/PhysicsLayer.h"
#include "Frost.h"
#include "Player.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Renderer/PostEffect/FogEffect.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include "Vehicles/Vehicle.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <Frost/Renderer/PostEffect/ToonEffect.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
PlayerSpringCameraScript::OnFixedUpdate(float deltaTime)
{
    ProcessInput(deltaTime);
    UpdateTPCam(deltaTime);
    UpdateSpringCam(deltaTime);
}

void
PlayerSpringCameraScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
    auto layer1 = Physics::GetBodyInterface().GetObjectLayer(params.inBody1.GetID());
    auto layer2 = Physics::GetBodyInterface().GetObjectLayer(params.inBody2.GetID());
    auto fog = springCam.GetComponent<Camera>().GetEffect<FogEffect>();
    if (layer1 == ObjectLayers::WATER || layer2 == ObjectLayers::WATER)
        fog->SetFog(waterFogMinDepth, waterFogStrength, waterFogColor);
    else
        fog->SetFog(skyFogMinDepth, skyFogStrength, skyFogColor);
}

// Warning : params may contains bodies that are not valid at the moment
void
PlayerSpringCameraScript::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
    auto fog = springCam.GetComponent<Camera>().GetEffect<FogEffect>();
    auto go1 = scene->GetGameObjectFromId(params.first);
    if (go1.HasComponent<RigidBody>())
    {
        auto bodyId1 = go1.GetComponent<RigidBody>().runtimeBodyID;
        auto layer1 = Physics::GetBodyInterface().GetObjectLayer(bodyId1);
        if (layer1 == ObjectLayers::WATER)
        {
            fog->SetFog(skyFogMinDepth, skyFogStrength, skyFogColor);
        }
    }

    auto go2 = scene->GetGameObjectFromId(params.second);
    if (go2.HasComponent<RigidBody>())
    {
        auto bodyId2 = go2.GetComponent<RigidBody>().runtimeBodyID;
        auto layer2 = Physics::GetBodyInterface().GetObjectLayer(bodyId2);
        if (layer2 == ObjectLayers::WATER)
        {
            fog->SetFog(skyFogMinDepth, skyFogStrength, skyFogColor);
        }
    }
};

void
PlayerSpringCameraScript::UpdateTPCam(float deltaTime)
{
    DirectX::XMVECTOR quaternion =
        DirectX::XMQuaternionRotationRollPitchYaw(_cameraPivotRotationX, _cameraPivotRotationY, 0.0f);

    auto& cameraPivotTransform = cameraPivot.GetComponent<Transform>();
    DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&cameraPivotTransform.rotation), quaternion);

    // TODO : cam rotation is actually one frame behind cameraPivot rotation
    auto& cameraPivotWTransform = cameraPivot.GetComponent<WorldTransform>();

    auto& springCamRB = springCam.GetComponent<RigidBody>();
    auto& playerWTransform = player.GetComponent<WorldTransform>();

    // Set camera pos according to player velocity
    auto& playerRB = player.GetComponent<RigidBody>();
    auto& thirdPersonCameraTransform = thirdPersonCamera.GetComponent<Transform>();

    auto velocity = Physics::Get().body_interface->GetLinearVelocity(playerRB.runtimeBodyID);
    auto speed = velocity.LengthSq();
    thirdPersonCameraTransform.position.z = defaultThirdPersonCameraDistance + speed * thirdPersonCamVelocityToDistance;
}

JPH::Quat
PlayerSpringCameraScript::LookAtQuaternion(const JPH::Vec3& cameraPos, const JPH::Vec3& targetPos)
{
    using namespace JPH;
    // Vector from camera to target
    Vec3 forward = (targetPos - cameraPos).Normalized();

    // World up vector
    Vec3 up = Vec3::sAxisY();

    // Compute right vector
    Vec3 right = up.Cross(forward).Normalized();

    // Recompute true up
    Vec3 newUp = forward.Cross(right);

    // Build rotation matrix (camera looks along +Z)
    Mat44 rot(Vec4(right, 0), Vec4(newUp, 0), Vec4(forward, 0), Vec4(0, 0, 0, 1));

    // Convert to quaternion
    return rot.GetQuaternion();
}

class IgnoreCameraLayerFilter : public JPH::ObjectLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inLayer) const override
    {
        return inLayer == ObjectLayers::NON_MOVING; // ignore la camera
    }
};

class RayCastBroadPhaseFilter : public JPH::BroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
    {
        return inLayer == Frost::BroadPhaseLayers::NON_MOVING;
    }
};

void
PlayerSpringCameraScript::UpdateSpringCam(float deltaTime)
{
    auto& playerWTransform = player.GetComponent<WorldTransform>();
    auto& thirdPersonCameraWTransform = thirdPersonCamera.GetComponent<WorldTransform>();
    auto& springCamRigidBody = springCam.GetComponent<RigidBody>();

    JPH::Vec3 cameraPos = Physics::Get().body_interface->GetPosition(springCamRigidBody.runtimeBodyID);
    JPH::Vec3 cameraVelocity = Physics::Get().body_interface->GetLinearVelocity(springCamRigidBody.runtimeBodyID);
    auto desiredPos = isThirdPerson ? Math::vector_cast<JPH::Vec3>(thirdPersonCameraWTransform.position)
                                    : Math::vector_cast<JPH::Vec3>(playerWTransform.position);

    auto playerPosition = Math::vector_cast<JPH::Vec3>(playerWTransform.position);

    // Make a getter inside player
    auto renderer = playerManager->GetCurrentVehicle().second->GetModelRendererObject();
    renderer.SetActive((playerPosition - cameraPos).Length() > playerCullingDistance);

    JPH::Vec3 displacement = desiredPos - cameraPos;
    JPH::Vec3 springForce = displacement * stiffness;

    auto playerBodyId = playerManager->GetCurrentVehicle().second->GetBodyID();
    springForce *= 1 / (1 + Physics::GetBodyInterface().GetAngularVelocity(playerBodyId).Length());
    auto newPos = cameraPos + deltaTime * springForce;

    if (!playerManager->forceSpecificCameraPos)
        Physics::Get().body_interface->SetPosition(
            springCamRigidBody.runtimeBodyID, newPos, JPH::EActivation::Activate);

    if (isThirdPerson)
    {
        JPH::RRayCast ray;
        ray.mOrigin = playerPosition;
        ray.mDirection = (newPos - playerPosition) * 1.1f;
        float desiredDistance = ray.mDirection.Length();

        RayCastBroadPhaseFilter bpFilter;
        IgnoreCameraLayerFilter objectFilter;

        JPH::RayCastResult result;
        if (desiredDistance > 0.01 &&
            Physics::Get().physics_system.GetNarrowPhaseQuery().CastRay(ray, result, bpFilter, objectFilter))
        {
            cameraPos = ray.GetPointOnRay(result.mFraction * 0.9f);
            Physics::Get().body_interface->SetPosition(
                springCamRigidBody.runtimeBodyID, cameraPos, JPH::EActivation::Activate);
        }
    }

    UpdateSpringCamRotation(newPos, deltaTime);
}

void
PlayerSpringCameraScript::UpdateSpringCamRotation(const JPH::Vec3& pos, float deltaTime)
{
    auto& playerWTransform = player.GetComponent<WorldTransform>();
    auto& springCamRigidBody = springCam.GetComponent<RigidBody>();
    auto springCamRot = Physics::Get().body_interface->GetRotation(springCamRigidBody.runtimeBodyID);
    auto newRot =
        LookAtQuaternion(pos,
                         Math::vector_cast<JPH::Vec3>(
                             playerWTransform.position) // +Physics::GetBodyInterface().GetLinearVelocity(playerBodyId)
        );
    auto rotLerpFactor = 0.75f;
    if (!playerManager->forceSpecificCameraPos)
        Physics::Get().body_interface->SetRotation(springCamRigidBody.runtimeBodyID,
                                                   newRot, //(rotLerpFactor * springCamRot + (1 - rotLerpFactor) *
                                                           // newRot).Normalized(),
                                                   JPH::EActivation::Activate);

    playerManager->forceSpecificCameraPos = false;
}

void
PlayerSpringCameraScript::ProcessInput(float deltaTime)
{
    EulerAngles pivotRot = Math::QuaternionToEulerAngles(cameraPivot.GetComponent<Transform>().rotation);

    auto* window = Application::GetWindow();
    uint32_t centerX = window->GetWidth() / 2;
    uint32_t centerY = window->GetHeight() / 2;

    isThirdPerson = !(Input::GetMouse().IsButtonHold(Mouse::MouseBoutton::Middle));

    if (Input::GetKeyboard().IsKeyPressed(K_C))
    {
        _freeCam = !_freeCam;

        if (_freeCam)
        {
            Input::GetMouse().HideCursor();
            Input::GetMouse().SetPosition({ centerX, centerY });
        }
        else
        {
            Input::GetMouse().ShowCursor();
        }

        return;
    }

    if (!_freeCam)
        return;

    auto currentMousePos = Input::GetMouse().GetPosition();

    int deltaX = static_cast<int>(currentMousePos.x) - static_cast<int>(centerX);
    int deltaY = static_cast<int>(currentMousePos.y) - static_cast<int>(centerY);

    if (deltaX != 0 || deltaY != 0)
    {
        _yaw += static_cast<float>(deltaX) * _mouseSensitivity;
        _pitch += static_cast<float>(deltaY) * _mouseSensitivity;

        Angle<Radian> upperLimit = 74.0_deg;
        Angle<Radian> lowerLimit = -99.0_deg;
        _pitch = std::clamp(_pitch, lowerLimit.value(), upperLimit.value());

        _cameraPivotRotationX = _pitch;
        _cameraPivotRotationY = _yaw;

        Input::GetMouse().SetPosition({ centerX, centerY });
    }

    // Switch cam to 3rd person or 1st person
    /*if (Input::GetKeyboard().GetKeyState(K_E) == KeyState::DOWN)
    {
            isThirdPerson = !isThirdPerson;
    }*/
}

PlayerCamera::PlayerCamera(Player* player) : _player{ player }
{
    using namespace JPH;
    auto& scene = Game::GetScene();

    // Camera Pivot
    _cameraPivot = scene.CreateGameObject("Camera Pivot", _player->GetPlayerID());
    _cameraPivot.AddComponent<Transform>(Vector3{ 0.0f, 0.0f, 0.0f });
    _cameraPivot.AddComponent<WorldTransform>();

    // 3rdPersVirtualCamera
    _3rdPersVirtCamera = scene.CreateGameObject("3rd Person Virtual Camera", _cameraPivot);
    _3rdPersVirtCamera.AddComponent<Transform>(Vector3{ 0.0f, 2, -10.0f });
    _3rdPersVirtCamera.AddComponent<WorldTransform>();
    // debug
    // scene.AddComponent<ModelRenderer>(_3rdPersVirtCamera,
    // "./resources/meshes/sphere.fbx");
    auto& tpCamWTransform = _3rdPersVirtCamera.GetComponent<WorldTransform>();

    // Camera
    _camera = scene.CreateGameObject("Camera");
    _camera.AddComponent<Transform>(Vector3{ 0.0f, 10, -20.0f });
    auto& camComponent = _camera.AddComponent<Camera>();
    camComponent.postEffects.push_back(std::make_shared<ToonEffect>());
    camComponent.postEffects.push_back(std::make_shared<FogEffect>());
    camComponent.postEffects.push_back(std::make_shared<RadialBlurEffect>());
    camComponent.postEffects.push_back(std::make_shared<ScreenShakeEffect>());
    camComponent.postEffects.push_back(std::make_shared<ChromaticAberrationEffect>());

    camComponent.backgroundColor.r = 47.0f / 255.0f;
    camComponent.backgroundColor.g = 116.0f / 255.0f;
    camComponent.backgroundColor.b = 228.0f / 255.0f;
    camComponent.backgroundColor.a = 1.0f;

    // Create the Camera Sensor
    auto& rb =
        _camera.AddComponent<RigidBody>(ShapeSphere{ 0.01f }, ObjectLayers::CAMERA, RigidBody::MotionType::Kinematic);
    rb.isSensor = true;

    _camera.AddScript<PlayerSpringCameraScript>(_cameraPivot, _3rdPersVirtCamera, _camera, _player);

    _cameraBodyID = _camera.GetComponent<RigidBody>().runtimeBodyID;
    _bodyInter = Physics::Get().body_interface;

    screenShake = camComponent.GetEffect<ScreenShakeEffect>().get();
    radialBlur = camComponent.GetEffect<RadialBlurEffect>().get();
    chromaticAberration = camComponent.GetEffect<ChromaticAberrationEffect>().get();
    chromaticAberration->SetCenterRed({ 0.292, 0.33 });
    chromaticAberration->SetCenterGreen({ 0.708, 0.307 });
    chromaticAberration->SetCenterBlue({ 0.51, 0.818 });
    chromaticAberration->SetStrength(0.0f);
}

void
PlayerCamera::Shake(float duration, float amplitude, ScreenShakeEffect::AttenuationType type)
{
    if (screenShake != nullptr && std::abs(duration) < _maxShakeDuration && std::abs(amplitude) < _maxShakeAmplitude)
        screenShake->Shake(duration, amplitude, type);
}

void
PlayerCamera::SetRadialBlurStrength(float strength)
{
    if (radialBlur != nullptr && std::abs(strength) < _maxRadialBlurStrength)
        radialBlur->SetStrength(strength);
}

void
PlayerCamera::SetChromaticAberrationStrength(float strength)
{
    if (chromaticAberration != nullptr)
        chromaticAberration->SetStrength(strength);
}
