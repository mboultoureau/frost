#include "Player/PlayerSpringCamera.h"
#include "Physics/PhysicLayer.h"
#include "GameState/GameState.h"

#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

#undef min
#undef max

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

        _cameraPivot = GetGameObject().GetChildByName("CameraPivot");
        if (!_cameraPivot.IsValid())
        {
            _cameraPivot = _playerController.GetChildByName("CameraPivot");
        }

        _desiredCameraLocation = _cameraPivot.GetChildByName("DesiredCameraLocation");

        FT_ASSERT(_player.IsValid(), "PlayerCamera script requires to be attached to the Player GameObject");
        FT_ASSERT(_playerController.IsValid(),
                  "PlayerCamera script requires a child GameObject named 'PlayerController'");
        FT_ASSERT(_camera.IsValid(), "PlayerCamera script requires a child GameObject named 'Camera'");
        FT_ASSERT(_cameraPivot.IsValid(), "PlayerCamera script requires a child GameObject named 'CameraPivot'");
        FT_ASSERT(_desiredCameraLocation.IsValid(),
                  "PlayerCamera script requires a child GameObject named 'DesiredCameraLocation' (can be nested)");

        // Get vehicle game objects
        _moto = _playerController.GetChildByName("Moto", true);
        _boat = _playerController.GetChildByName("Boat", true);
        _plane = _playerController.GetChildByName("Plane", true);

        // Get post effects
        auto& camComponent = _camera.GetComponent<Camera>();
        camComponent.postEffects.push_back(std::make_shared<ToonEffect>());
        camComponent.postEffects.push_back(std::make_shared<FogEffect>());
        camComponent.postEffects.push_back(std::make_shared<RadialBlurEffect>());
        camComponent.postEffects.push_back(std::make_shared<ScreenShakeEffect>());
        camComponent.postEffects.push_back(std::make_shared<ColorCorrectionEffect>());

        _radialBlur = camComponent.GetEffect<RadialBlurEffect>().get();
        _screenShake = camComponent.GetEffect<ScreenShakeEffect>().get();

        if (_desiredCameraLocation.IsValid())
        {
            auto& trans = _desiredCameraLocation.GetComponent<Transform>();
            _baseThirdPersonDistance = trans.position.z;
        }
    }

    void PlayerSpringCamera::OnFixedUpdate(float fixedDeltaTime)
    {
        auto& camComponent = _camera.GetComponent<Camera>();
        if (!GameState::Get().GetPlayerData(_player).isInToon)
        {
            camComponent.GetEffect<ToonEffect>()->SetEnabled(false);
        }
        else
        {
            camComponent.GetEffect<ToonEffect>()->SetEnabled(true);
        }
        if (!GameState::Get().GetPlayerData(_player).isCameraInWater)
        {
            camComponent.GetEffect<FogEffect>()->SetEnabled(false);
        }
        else
        {
            camComponent.GetEffect<FogEffect>()->SetEnabled(true);
        }

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
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto& controllerTrans = _playerController.GetComponent<Transform>();
        auto& pivotTrans = _cameraPivot.GetComponent<Transform>();
        auto& playerRB = _playerController.GetComponent<RigidBody>();

        pivotTrans.position = controllerTrans.position + Vector3(0, 1.5f, 0);

        JPH::Quat playerPhysicsRot = Physics::Get().body_interface->GetRotation(playerRB.runtimeBodyID);

        JPH::Quat mouseRot = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), _cameraPivotRotationY) *
                             JPH::Quat::sRotation(JPH::Vec3::sAxisX(), _cameraPivotRotationX);

        JPH::Quat targetRot = playerPhysicsRot * mouseRot;

        JPH::Quat currentPivotRot = Math::vector_cast<JPH::Quat>(pivotTrans.rotation);

        JPH::Quat smoothedRot =
            currentPivotRot.SLERP(targetRot, std::min(1.0f, fixedDeltaTime * _rotationSmoothingSpeed));

        pivotTrans.rotation = Math::vector_cast<Math::Vector4>(smoothedRot);

        auto& thirdPersonCameraTransform = _desiredCameraLocation.GetComponent<Transform>();
        auto velocity = Physics::Get().body_interface->GetLinearVelocity(playerRB.runtimeBodyID);
        auto speed = velocity.Length();

        thirdPersonCameraTransform.position.z = _baseThirdPersonDistance + speed * _thirdPersonCamVelocityToDistance;
    }

    void PlayerSpringCamera::_UpdateSpringCam(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto& playerWTransform = _playerController.GetComponent<WorldTransform>();
        auto& thirdPersonCameraWTransform = _desiredCameraLocation.GetComponent<WorldTransform>();
        auto& springCamRigidBody = _camera.GetComponent<RigidBody>();
        auto& pivotWTransform = _cameraPivot.GetComponent<WorldTransform>();

        JPH::Vec3 cameraPos = Physics::Get().body_interface->GetPosition(springCamRigidBody.runtimeBodyID);
        JPH::Vec3 cameraVelocity = Physics::Get().body_interface->GetLinearVelocity(springCamRigidBody.runtimeBodyID);

        auto desiredPos = isThirdPerson ? Math::vector_cast<JPH::Vec3>(thirdPersonCameraWTransform.position)
                                        : Math::vector_cast<JPH::Vec3>(pivotWTransform.position);

        auto playerPosition = Math::vector_cast<JPH::Vec3>(playerWTransform.position);

        bool active = (playerPosition - cameraPos).Length() > _playerCullingDistance;
        auto& playerData = GameState::Get().GetPlayerData(_player);
        switch (playerData.currentVehicle)
        {
            case VehicleType::MOTO:
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

        JPH::BodyID playerBodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        springForce *= 1.0f / (1.0f + 0.1f * Physics::GetBodyInterface().GetAngularVelocity(playerBodyId).Length());
        springForce -= cameraVelocity * 2.0f;

        auto newPos = cameraPos + fixedDeltaTime * springForce;

        if (!playerData.forceSpecificCameraPos)
            Physics::Get().body_interface->SetPosition(
                springCamRigidBody.runtimeBodyID, newPos, JPH::EActivation::Activate);

        if (isThirdPerson)
        {
            JPH::RRayCast ray;
            auto pivotPos = Math::vector_cast<JPH::Vec3>(pivotWTransform.position);
            ray.mOrigin = pivotPos;
            ray.mDirection = (newPos - pivotPos) * 1.1f;
            float desiredDistance = ray.mDirection.Length();

            RayCastBroadPhaseFilter bpFilter;
            IgnoreCameraLayerFilter objectFilter;

            JPH::RayCastResult result;
            if (desiredDistance > 0.01 &&
                Physics::Get().physics_system.GetNarrowPhaseQuery().CastRay(ray, result, bpFilter, objectFilter))
            {
                cameraPos = ray.GetPointOnRay(result.mFraction * 0.95f);
                Physics::Get().body_interface->SetPosition(
                    springCamRigidBody.runtimeBodyID, cameraPos, JPH::EActivation::Activate);
            }
        }

        _UpdateSpringCamRotation(newPos, fixedDeltaTime);
    }

    void PlayerSpringCamera::_UpdateSpringCamRotation(const JPH::Vec3& pos, float fixedDeltaTime)
    {
        auto& playerData = GameState::Get().GetPlayerData(_player);
        auto& springCamRigidBody = _camera.GetComponent<RigidBody>();
        auto& playerWTransform = _playerController.GetComponent<WorldTransform>();
        auto& pivotWTransform = _cameraPivot.GetComponent<WorldTransform>();

        JPH::Quat pivotRot = Math::vector_cast<JPH::Quat>(pivotWTransform.rotation);
        JPH::Vec3 pivotUp = pivotRot.RotateAxisY();
        JPH::Vec3 targetPos = Math::vector_cast<JPH::Vec3>(playerWTransform.position) + pivotUp * 1.5f;

        auto newRot = _LookAtQuaternion(pos, targetPos, pivotUp);

        if (!playerData.forceSpecificCameraPos)
            Physics::Get().body_interface->SetRotation(
                springCamRigidBody.runtimeBodyID, newRot, JPH::EActivation::Activate);

        playerData.forceSpecificCameraPos = false;
    }

    JPH::Quat PlayerSpringCamera::_LookAtQuaternion(const JPH::Vec3& cameraPos,
                                                    const JPH::Vec3& targetPos,
                                                    const JPH::Vec3& up)
    {
        using namespace JPH;

        Vec3 forward = (targetPos - cameraPos).Normalized();

        if (forward.LengthSq() < 0.001f)
            return JPH::Quat::sIdentity();

        Vec3 right = up.Cross(forward).Normalized();

        if (right.LengthSq() < 0.001f)
        {
            right = Vec3::sAxisX();
        }

        Vec3 newUp = forward.Cross(right);

        Mat44 rot(Vec4(right, 0), Vec4(newUp, 0), Vec4(forward, 0), Vec4(0, 0, 0, 1));
        return rot.GetQuaternion();
    }
} // namespace GameLogic