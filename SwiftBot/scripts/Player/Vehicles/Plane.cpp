#include "Plane.h"
#include "Physics/PhysicLayer.h"
#include <Frost/Scene/Components/RigidBody.h>
#include <Frost/Scene/Components/WorldTransform.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#undef max
#undef min

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

    Plane::Plane(GameObject player) : Vehicle(player)
    {
        _vehicle = player.GetChildByName("Plane", true);
        _currentSpeed = _baseForwardSpeed;
    }

    void Plane::Show()
    {
        _vehicle.SetActive(true);

        const auto& transform = _playerController.GetComponent<Transform>();
        Vec3 pos = Math::vector_cast<Vec3>(transform.position);
        Quat rot = Math::vector_cast<Quat>(transform.rotation);

        RefConst<Shape> shape = new SphereShape(1.0f);
        BodyCreationSettings bodySettings(shape, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
        bodySettings.mRestitution = 0.5f;
        bodySettings.mAllowSleeping = false;
        bodySettings.mFriction = 130.0f;
        bodySettings.mGravityFactor = 0.2f;
        bodySettings.mAllowedDOFs =
            EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;

        Body* body = Physics::CreateBody(bodySettings);
        BodyID bodyId = body->GetID();
        Physics::AddBody(bodyId, EActivation::Activate);

        Physics::GetBodyInterface().SetUserData(bodyId, static_cast<JPH::uint64>(_playerController.GetHandle()));
        auto& rigidBody = _playerController.AddComponent<RigidBody>(bodyId);
        rigidBody.objectLayer = ObjectLayers::PLAYER;
        rigidBody.motionType = RigidBody::MotionType::Dynamic;

        _justAppeared = true;
        _inContinuousCollision = false;

        _UpdateInternalStateFromBody(false);
    }

    void Plane::Hide()
    {
        _vehicle.SetActive(false);
        if (_playerController.HasComponent<RigidBody>())
        {
            Physics::RemoveAndDestroyBody(_playerController.GetComponent<RigidBody>().runtimeBodyID);
            _playerController.RemoveComponent<RigidBody>();
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

        if (_inContinuousCollision)
        {
            if (_collisionCoolDownTimer.GetDuration() > _collisionCoolDownDuration)
            {
                _inContinuousCollision = false;
                _UpdateInternalStateFromBody(true);
            }
            else
            {
                return;
            }
        }

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& bodyInterface = Physics::GetBodyInterface();

        if (_justAppeared)
            _UpdateInternalStateFromBody(false);

        // --- Flight Model ---
        JPH::Vec3 vel = bodyInterface.GetLinearVelocity(bodyId);
        JPH::Vec3 horizontalVel = { vel.GetX(), 0, vel.GetZ() };

        _currentSpeed = horizontalVel.Length();
        if (_justAppeared && _currentSpeed < 1.0f)
            _currentSpeed = _baseForwardSpeed;

        float targetPitch = -_upDownInput * _maxPitchAngle;
        float targetRoll = -_leftRightInput * _maxRollAngle;

        _currentPitch = MoveTowards(_currentPitch, targetPitch, _pitchSpeed * fixedDeltaTime);
        _currentRoll = MoveTowards(_currentRoll, targetRoll, _rollSpeed * fixedDeltaTime);

        // Le Yaw tourne normalement
        _currentYaw -= _currentRoll * _yawFromRoll * fixedDeltaTime;

        // Gestion de la vitesse
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

        // Construction de la rotation (Standard, Yaw 0 = Z+)
        Quat yawQuat = Quat::sRotation(Vec3::sAxisY(), _currentYaw);
        Quat pitchQuat = Quat::sRotation(Vec3::sAxisX(), _currentPitch);
        Quat rollQuat = Quat::sRotation(Vec3::sAxisZ(), _currentRoll);
        Quat newRotation = yawQuat * pitchQuat * rollQuat;

        // Calcul du vecteur direction Horizontal
        // Utilisation stricte de l'ancienne méthode (Sin/Cos du Yaw)
        // FIX: On INVERSE le vecteur (-Vec3) pour aller vers Z- (là où regarde le mesh)
        JPH::Vec3 horizontalDir = -JPH::Vec3(std::sin(_currentYaw), 0, std::cos(_currentYaw));

        // Application de la vitesse
        JPH::Vec3 finalVelocity = horizontalDir * _currentSpeed + JPH::Vec3(0, verticalRate, 0);

        bodyInterface.SetRotation(bodyId, newRotation, EActivation::Activate);
        bodyInterface.SetLinearVelocity(bodyId, finalVelocity);

        _justAppeared = false;
    }

    void Plane::_UpdateInternalStateFromBody(bool accountForVelocity)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;
        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& bodyInterface = Physics::GetBodyInterface();

        Quat rot = bodyInterface.GetRotation(bodyId);
        Vec3 speed = bodyInterface.GetLinearVelocity(bodyId);
        Vec3 fwd;

        if (accountForVelocity && speed.Length() > _speedAcknowledgementThreshold)
        {
            // FIX: On inverse la vitesse lue (-speed) pour retrouver le "Forward" logique du mesh
            // Si l'avion va vers Z- (arrière physique), c'est qu'il va tout droit visuellement.
            fwd = -speed.NormalizedOr(rot.RotateAxisZ());
        }
        else
        {
            fwd = rot.RotateAxisZ();
        }

        _currentYaw = std::atan2(fwd.GetX(), fwd.GetZ());

        Vec3 up = rot.RotateAxisY();
        _currentPitch = std::asin(-fwd.GetY());

        Vec3 right = rot.RotateAxisX();
        _currentRoll = std::atan2(right.GetY(), up.GetY());
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