#include "Plane.h"
#include "Physics/PhysicLayer.h"
#include <Frost/Scene/Components/RigidBody.h>
#include <Frost/Scene/Components/WorldTransform.h>
#include <Frost/Math/Math.h>

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

        // On force une mise � jour imm�diate pour caler le Yaw
        _UpdateInternalStateFromBody(false);
    }

    void Plane::Hide()
    {
        _vehicle.SetActive(false);
        if (_playerController.HasComponent<RigidBody>())
            _playerController.RemoveComponent<RigidBody>();
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

        // On r�cup�re la vitesse. Si on vient de spawner, on s'assure d'avoir la vitesse de base
        _currentSpeed = horizontalVel.Length();
        if (_justAppeared && _currentSpeed < 1.0f)
            _currentSpeed = _baseForwardSpeed;

        float targetPitch = -_upDownInput * _maxPitchAngle;
        float targetRoll = -_leftRightInput * _maxRollAngle;

        _currentPitch = MoveTowards(_currentPitch, targetPitch, _pitchSpeed * fixedDeltaTime);
        _currentRoll = MoveTowards(_currentRoll, targetRoll, _rollSpeed * fixedDeltaTime);

        // Note: Inversion possible ici aussi selon le sens de rotation voulu
        _currentYaw -= _currentRoll * _yawFromRoll * fixedDeltaTime;

        // Gestion de la vitesse (W = piqu� = acc�l�ration)
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

        // Construction de la rotation
        // FIX : On ajoute JPH_PI (180�) au Yaw pour retourner la logique physique
        Quat yawQuat = Quat::sRotation(Vec3::sAxisY(), _currentYaw + JPH::JPH_PI);
        Quat pitchQuat = Quat::sRotation(Vec3::sAxisX(), _currentPitch);
        Quat rollQuat = Quat::sRotation(Vec3::sAxisZ(), _currentRoll);
        Quat newRotation = yawQuat * pitchQuat * rollQuat;

        // Calcul du vecteur direction bas� sur la rotation calcul�e
        // On prend l'axe Z de la rotation finale pour �tre s�r d'aller "tout droit"
        Vec3 forwardDir = newRotation.RotateAxisZ();

        // Application de la vitesse
        Vec3 finalVelocity = forwardDir * _currentSpeed + Vec3(0, verticalRate, 0);

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
            fwd = speed.NormalizedOr(rot.RotateAxisZ());
        else
            fwd = rot.RotateAxisZ();

        // Calcul du Yaw standard
        _currentYaw = std::atan2(fwd.GetX(), fwd.GetZ());

        // FIX : Si on d�tecte qu'on est invers� par rapport au mod�le, on compense ici aussi
        // Si l'avion part en arri�re, c'est que l'avant physique est l'arri�re visuel.
        // On d�cale de 180� (PI) pour s'aligner.
        _currentYaw += JPH::JPH_PI;

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