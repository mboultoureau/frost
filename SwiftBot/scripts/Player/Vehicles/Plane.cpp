#include "Player/Vehicles/Plane.h"
#include "Physics/PhysicLayer.h"
#include "GameState/GameState.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/MotionProperties.h>

#undef min
#undef max

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    static float MoveTowards(float current, float target, float maxDelta)
    {
        float diff = target - current;
        if (std::abs(diff) <= maxDelta)
            return target;
        return current + std::copysign(maxDelta, diff);
    }

    void Plane::_UpdateInternalStateFromBody(const JPH::Body& body, bool accountForVelocity)
    {
        Quat rot = body.GetRotation();
        Vec3 speed = body.GetLinearVelocity();
        Vec3 fwd;

        if (accountForVelocity && speed.Length() > _speedAcknowledgementThreshold)
            fwd = speed.NormalizedOr(rot.RotateAxisZ());
        else
            fwd = rot.RotateAxisZ();

        _currentYaw = std::atan2(fwd.GetX(), fwd.GetZ());
        _currentYaw += JPH::JPH_PI;

        Vec3 up = rot.RotateAxisY();
        _currentPitch = std::asin(-fwd.GetY());

        Vec3 right = rot.RotateAxisX();
        _currentRoll = std::atan2(right.GetY(), up.GetY());
    }

    Plane::Plane(GameObject player) : Vehicle(player)
    {
        _vehicle = player.GetChildByName("Plane", true);
        _currentSpeed = 0.0f;
    }

    void Plane::Show()
    {
        _vehicle.SetActive(true);

        const auto& transform = _playerController.GetComponent<Transform>();
        Vec3 pos = Math::vector_cast<Vec3>(transform.position);
        Quat rot = Math::vector_cast<Quat>(transform.rotation);

        RefConst<Shape> shape = new SphereShape(1.0f);
        BodyCreationSettings bodySettings(shape, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
        bodySettings.mRestitution = 3.0f;
        bodySettings.mAllowSleeping = false;
        bodySettings.mFriction = 130.0f;
        bodySettings.mGravityFactor = 0.2f;
        bodySettings.mAllowedDOFs =
            EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;

        bodySettings.mUserData = static_cast<JPH::uint64>(_playerController.GetHandle());

        Body* body = Physics::CreateBody(bodySettings);
        BodyID bodyId = body->GetID();
        Physics::AddBody(bodyId, EActivation::Activate);

        auto& rigidBody = _playerController.AddComponent<RigidBody>(bodyId);
        rigidBody.objectLayer = ObjectLayers::PLAYER;
        rigidBody.motionType = RigidBody::MotionType::Dynamic;

        _justAppeared = true;
        _inContinuousCollision = false;

        BodyLockWrite lock(Physics::Get().physics_system.GetBodyLockInterface(), bodyId);
        if (lock.Succeeded())
        {
            _UpdateInternalStateFromBody(lock.GetBody(), false);
        }
    }

    void Plane::Hide()
    {
        _vehicle.SetActive(false);
        if (_playerController.HasComponent<RigidBody>())
        {
            BodyID bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
            _playerController.RemoveComponent<RigidBody>();
            Physics::RemoveAndDestroyBody(bodyId);
        }
    }

    void Plane::OnMove(float right, float forward)
    {
        _leftRightInput = right;
        _upDownInput = -forward;
    }

    void Plane::OnBrake(bool brake) {}

    void Plane::OnSpecialAction(bool special) {}

    void Plane::OnCollisionEnter(const Frost::Math::Vector3& contactNormal)
    {
        if (!_inContinuousCollision)
        {
            auto& camera = _camera.GetComponent<Camera>();
            if (auto shake = camera.GetEffect<ScreenShakeEffect>())
            {
                shake->Shake(
                    0.4f, _currentSpeed * _screenShakeSpeedMultiplier, ScreenShakeEffect::AttenuationType::EaseOut);
            }
        }
        _inContinuousCollision = true;
        _collisionCoolDownTimer.Start();
    }

    void Plane::OnPreFixedUpdate(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto& playerData = GameState::Get().GetPlayerData(_playerController.GetParent());
        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& bodyInterface = Physics::GetBodyInterface();

        bool applyFlightModel = true;

        {
            BodyLockWrite lock(Physics::Get().physics_system.GetBodyLockInterface(), bodyId);
            if (!lock.Succeeded())
                return;
            auto& body = lock.GetBody();

            if (playerData.isInWater)
            {
                applyFlightModel = false;
                if (auto* mp = body.GetMotionProperties())
                {
                    mp->SetGravityFactor(0.0f);
                    mp->SetLinearDamping(0.8f);
                    mp->SetAngularDamping(0.8f);
                }
                body.SetLinearVelocity(body.GetLinearVelocity() * 0.98f);
                body.SetAngularVelocity(body.GetAngularVelocity() * 0.98f);
                _inContinuousCollision = false;
            }
            else
            {
                if (auto* mp = body.GetMotionProperties())
                {
                    mp->SetGravityFactor(0.2f);
                    mp->SetLinearDamping(0.05f);
                    mp->SetAngularDamping(0.05f);
                }

                if (_inContinuousCollision)
                {
                    if (_collisionCoolDownTimer.GetDuration() > _collisionCoolDownDuration)
                    {
                        _inContinuousCollision = false;
                        _UpdateInternalStateFromBody(body, true);
                    }
                    else
                    {
                        applyFlightModel = false;
                    }
                }

                if (_justAppeared)
                {
                    _UpdateInternalStateFromBody(body, false);
                }
            }
        }

        if (applyFlightModel)
        {
            float targetPitch = -_upDownInput * _maxPitchAngle;
            float targetRoll = -_leftRightInput * _maxRollAngle;

            _currentPitch = MoveTowards(_currentPitch, targetPitch, _pitchSpeed * fixedDeltaTime);
            _currentRoll = MoveTowards(_currentRoll, targetRoll, _rollSpeed * fixedDeltaTime);
            _currentYaw -= _currentRoll * _yawFromRoll * fixedDeltaTime;

            if (_upDownInput < 0)
            {
                float targetSpeed = _maxForwardSpeed;
                float speedLerp = 1.0f - std::exp(-_downwardSpeedSmoothing * fixedDeltaTime);
                _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
            }
            else if (_upDownInput > 0)
            {
                float targetSpeed = _minForwardSpeed;
                float speedLerp = 1.0f - std::exp(-_upwardSpeedSmoothing * fixedDeltaTime);
                _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
            }
            else
            {
                float targetSpeed = _baseForwardSpeed;
                if (_currentSpeed > targetSpeed)
                    _currentSpeed = std::max(targetSpeed, _currentSpeed - _noInputDeceleration * fixedDeltaTime);
                else if (_currentSpeed < targetSpeed)
                    _currentSpeed = std::min(targetSpeed, _currentSpeed + _noInputDeceleration * fixedDeltaTime);
            }
            _currentSpeed = std::clamp(_currentSpeed, _minForwardSpeed, _maxForwardSpeed);

            float verticalRate = -_baseSinkRate;
            if (_upDownInput < 0)
                verticalRate = -_diveSinkRate;
            else if (_upDownInput > 0)
                verticalRate = (_currentSpeed / _maxForwardSpeed) * _climbLiftRateOffset;

            Quat yawQuat = Quat::sRotation(Vec3::sAxisY(), _currentYaw + JPH::JPH_PI);
            Quat pitchQuat = Quat::sRotation(Vec3::sAxisX(), _currentPitch);
            Quat rollQuat = Quat::sRotation(Vec3::sAxisZ(), _currentRoll);
            Quat newRotation = yawQuat * pitchQuat * rollQuat;

            Vec3 forwardDir = newRotation.RotateAxisZ();
            Vec3 finalVelocity = forwardDir * _currentSpeed + Vec3(0, verticalRate, 0);

            bodyInterface.SetRotation(bodyId, newRotation, EActivation::Activate);
            bodyInterface.SetLinearVelocity(bodyId, finalVelocity);

            _justAppeared = false;
        }
    }

    void Plane::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto& camera = _camera.GetComponent<Camera>();
        if (auto radialBlur = camera.GetEffect<RadialBlurEffect>())
            radialBlur->SetStrength(_currentSpeed * _radialBlurSpeedFactor);
    }
} // namespace GameLogic