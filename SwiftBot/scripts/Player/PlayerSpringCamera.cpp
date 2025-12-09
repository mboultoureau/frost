#include "Player/PlayerSpringCamera.h"
#include "Physics/PhysicLayer.h"
#include "GameState/GameState.h"

#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    class IgnoreCameraLayerFilter : public JPH::ObjectLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer) const override { return inLayer == ObjectLayers::NON_MOVING; }
    };

    class RayCastBroadPhaseFilter : public JPH::BroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
        {
            return inLayer == Frost::BroadPhaseLayers::NON_MOVING;
        }
    };

    void PlayerSpringCamera::OnCreate()
    {
        // Get camera game objects
        _player = GetGameObject();
        _playerController = GetGameObject().GetChildByName("PlayerController");
        _camera = GetGameObject().GetChildByName("Camera");
        _cameraPivot = GetGameObject().GetChildByName("CameraPivot", true);
        _desiredCameraLocation = GetGameObject().GetChildByName("DesiredCameraLocation", true);

        FT_ASSERT(_player.IsValid(), "PlayerCamera script requires to be attached to the Player GameObject");
        FT_ASSERT(_playerController.IsValid(),
                  "PlayerCamera script requires a child GameObject named 'PlayerController'");
        FT_ASSERT(_camera.IsValid(), "PlayerCamera script requires a child GameObject named 'Camera'");
        FT_ASSERT(_cameraPivot.IsValid(),
                  "PlayerCamera script requires a child GameObject named 'CameraPivot' (can be nested)");
        FT_ASSERT(_desiredCameraLocation.IsValid(),
                  "PlayerCamera script requires a child GameObject named 'DesiredCameraLocation' (can be nested)");

        // Get vehicle game objects
        _moto = _player.GetChildByName("Moto", true);
        _boat = _player.GetChildByName("Boat", true);
        _plane = _player.GetChildByName("Plane", true);

        FT_ASSERT(_moto.IsValid(),
                  "PlayerSpringCamera script requires a child GameObject named 'Bike' (can be nested)");
        FT_ASSERT(_boat.IsValid(),
                  "PlayerSpringCamera script requires a child GameObject named 'Boat' (can be nested)");
        FT_ASSERT(_plane.IsValid(),
                  "PlayerSpringCamera script requires a child GameObject named 'Plane' (can be nested)");

        // Get post effects
        auto& camComponent = _camera.GetComponent<Camera>();
        _radialBlur = camComponent.GetEffect<RadialBlurEffect>().get();
        _screenShake = camComponent.GetEffect<ScreenShakeEffect>().get();
    }

    void PlayerSpringCamera::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!GameState::Get().IsInitialized())
        {
            return;
        }

        _ProcessInput(fixedDeltaTime);
        _UpdateTPCam(fixedDeltaTime);
        _UpdateSpringCam(fixedDeltaTime);
    }

    void PlayerSpringCamera::_ProcessInput(float fixedDeltaTime)
    {
        EulerAngles pivotRot = Math::QuaternionToEulerAngles(_cameraPivot.GetComponent<Transform>().rotation);

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
    }

    void PlayerSpringCamera::_UpdateTPCam(float fixedDeltaTime)
    {
        DirectX::XMVECTOR quaternion =
            DirectX::XMQuaternionRotationRollPitchYaw(_cameraPivotRotationX, _cameraPivotRotationY, 0.0f);

        auto& cameraPivotTransform = _cameraPivot.GetComponent<Transform>();
        DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&cameraPivotTransform.rotation), quaternion);

        // TODO : cam rotation is actually one frame behind cameraPivot rotation
        auto& cameraPivotWTransform = _cameraPivot.GetComponent<WorldTransform>();

        auto& springCamRB = _camera.GetComponent<RigidBody>();
        auto& playerWTransform = _playerController.GetComponent<WorldTransform>();

        auto& playerRB = _playerController.GetComponent<RigidBody>();
        auto& thirdPersonCameraTransform = _desiredCameraLocation.GetComponent<Transform>();

        auto velocity = Physics::Get().body_interface->GetLinearVelocity(playerRB.runtimeBodyID);
        auto speed = velocity.LengthSq();
        thirdPersonCameraTransform.position.z =
            _defaultThirdPersonCameraDistance + speed * _thirdPersonCamVelocityToDistance;
    }

    void PlayerSpringCamera::_UpdateSpringCam(float fixedDeltaTime)
    {
        auto& playerWTransform = _playerController.GetComponent<WorldTransform>();
        auto& thirdPersonCameraWTransform = _desiredCameraLocation.GetComponent<WorldTransform>();
        auto& springCamRigidBody = _camera.GetComponent<RigidBody>();

        JPH::Vec3 cameraPos = Physics::Get().body_interface->GetPosition(springCamRigidBody.runtimeBodyID);
        JPH::Vec3 cameraVelocity = Physics::Get().body_interface->GetLinearVelocity(springCamRigidBody.runtimeBodyID);
        auto desiredPos = isThirdPerson ? Math::vector_cast<JPH::Vec3>(thirdPersonCameraWTransform.position)
                                        : Math::vector_cast<JPH::Vec3>(playerWTransform.position);

        auto playerPosition = Math::vector_cast<JPH::Vec3>(playerWTransform.position);

        // Active or not renderer according to distance
        bool active = (playerPosition - cameraPos).Length() > _playerCullingDistance;
        auto& playerData = GameState::Get().GetPlayerData(_player);
        switch (playerData.currentVehicle)
        {
            case VehicleType::BIKE:
                _moto.SetActive(active);
                break;

            case VehicleType::BOAT:
                _boat.SetActive(active);
                break;

            case VehicleType::PLANE:
                _plane.SetActive(active);
                break;
        }

        JPH::Vec3 displacement = desiredPos - cameraPos;
        JPH::Vec3 springForce = displacement * _stiffness;

        JPH::BodyID playerBodyId;
        switch (playerData.currentVehicle)
        {
            case VehicleType::BIKE:
                // playerBodyId = _moto.GetComponent<RigidBody>().runtimeBodyID;
                return;
                break;
            case VehicleType::BOAT:
                playerBodyId = _boat.GetComponent<RigidBody>().runtimeBodyID;
                break;
            case VehicleType::PLANE:
                playerBodyId = _plane.GetComponent<RigidBody>().runtimeBodyID;
                break;
            default:
                playerBodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
                break;
        }

        springForce *= 1 / (1 + Physics::GetBodyInterface().GetAngularVelocity(playerBodyId).Length());
        auto newPos = cameraPos + fixedDeltaTime * springForce;

        if (!playerData.forceSpecificCameraPos)
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

        _UpdateSpringCamRotation(newPos, fixedDeltaTime);
    }

    void PlayerSpringCamera::_UpdateSpringCamRotation(const JPH::Vec3& pos, float fixedDeltaTime)
    {
        auto& playerData = GameState::Get().GetPlayerData(_player);
        auto& playerWTransform = _playerController.GetComponent<WorldTransform>();
        auto& springCamRigidBody = _camera.GetComponent<RigidBody>();
        auto springCamRot = Physics::Get().body_interface->GetRotation(springCamRigidBody.runtimeBodyID);
        auto newRot = _LookAtQuaternion(
            pos,
            Math::vector_cast<JPH::Vec3>(
                playerWTransform.position) // +Physics::GetBodyInterface().GetLinearVelocity(playerBodyId)
        );
        auto rotLerpFactor = 0.75f;
        if (!playerData.forceSpecificCameraPos)
            Physics::Get().body_interface->SetRotation(springCamRigidBody.runtimeBodyID,
                                                       newRot, //(rotLerpFactor * springCamRot + (1 - rotLerpFactor) *
                                                               // newRot).Normalized(),
                                                       JPH::EActivation::Activate);

        playerData.forceSpecificCameraPos = false;
    }

    JPH::Quat PlayerSpringCamera::_LookAtQuaternion(const JPH::Vec3& cameraPos, const JPH::Vec3& targetPos)
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
} // namespace GameLogic
