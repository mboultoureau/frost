#include "Player/Vehicles/Plane.h"
#include "Physics/PhysicLayer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#undef min
#undef max

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    // Fonction utilitaire pour s'approcher en douceur d'une valeur cible
    static float MoveTowards(float current, float target, float maxDelta)
    {
        float diff = target - current;
        if (std::abs(diff) <= maxDelta)
            return target;
        return current + std::copysign(maxDelta, diff);
    }

    Plane::Plane(GameObject player) : Vehicle(player)
    {
        // Trouver la représentation visuelle de l'avion
        _vehicle = player.GetChildByName("Plane", true);
        // Définir la vitesse initiale
        _currentSpeed = _baseForwardSpeed;
    }

    void Plane::Show()
    {
        _vehicle.SetActive(true);

        const auto& transform = _playerController.GetComponent<Transform>();
        Vec3 pos = Math::vector_cast<Vec3>(transform.position);
        Quat rot = Math::vector_cast<Quat>(transform.rotation);

        // Créer les paramètres du corps physique
        RefConst<Shape> shape = new SphereShape(1.0f);
        BodyCreationSettings bodySettings(shape, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
        bodySettings.mRestitution = 0.5f;
        bodySettings.mAllowSleeping = false;
        bodySettings.mFriction = 130.0f;
        bodySettings.mGravityFactor = 0.2f;
        bodySettings.mAllowedDOFs =
            EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;

        // Définir les données utilisateur pour lier le corps au GameObject
        bodySettings.mUserData = static_cast<JPH::uint64>(_playerController.GetHandle());

        // Créer le corps et l'ajouter au monde
        Body* body = Physics::CreateBody(bodySettings);
        BodyID bodyId = body->GetID();
        Physics::AddBody(bodyId, EActivation::Activate);

        // Ajouter le composant RigidBody au GameObject du joueur
        auto& rigidBody = _playerController.AddComponent<RigidBody>(bodyId);
        rigidBody.objectLayer = ObjectLayers::PLAYER;
        rigidBody.motionType = RigidBody::MotionType::Dynamic;

        // Initialiser les indicateurs d'état
        _justAppeared = true;
        _inContinuousCollision = false;

        // Forcer une mise à jour immédiate pour aligner correctement l'angle de lacet (Yaw)
        _UpdateInternalStateFromBody(false);
    }

    void Plane::Hide()
    {
        _vehicle.SetActive(false);
        if (_playerController.HasComponent<RigidBody>())
        {
            // Obtenir l'ID avant de retirer le composant
            BodyID bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
            // Retirer le composant de l'entité
            _playerController.RemoveComponent<RigidBody>();
            // Retirer et détruire le corps physique de la simulation
            Physics::RemoveAndDestroyBody(bodyId);
        }
    }

    void Plane::OnMove(float right, float forward)
    {
        _leftRightInput = right;
        _upDownInput = -forward; // Inverser l'entrée avant pour le contrôle du tangage (Pitch)
    }

    void Plane::OnBrake(bool brake)
    {
        // Non utilisé par l'avion
    }

    void Plane::OnSpecialAction(bool special)
    {
        // Non utilisé par l'avion
    }

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

        // Gérer le temps de recharge de la collision
        if (_inContinuousCollision)
        {
            if (_collisionCoolDownTimer.GetDuration() > _collisionCoolDownDuration)
            {
                _inContinuousCollision = false;
                // Resynchroniser l'état interne avec le corps physique après la fin de la collision
                _UpdateInternalStateFromBody(true);
            }
            else
            {
                // Ne pas appliquer le modèle de vol en état de collision continue
                return;
            }
        }

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& bodyInterface = Physics::GetBodyInterface();

        if (_justAppeared)
            _UpdateInternalStateFromBody(false);

        // --- Modèle de vol ---
        JPH::Vec3 vel = bodyInterface.GetLinearVelocity(bodyId);
        JPH::Vec3 horizontalVel = { vel.GetX(), 0, vel.GetZ() };

        // Récupérer la vitesse actuelle. Si on vient d'apparaître, s'assurer d'avoir la vitesse de base.
        _currentSpeed = horizontalVel.Length();
        if (_justAppeared && _currentSpeed < 1.0f)
            _currentSpeed = _baseForwardSpeed;

        // Calculer les angles de tangage et de roulis cibles à partir des entrées
        float targetPitch = -_upDownInput * _maxPitchAngle;
        float targetRoll = -_leftRightInput * _maxRollAngle;

        // Se déplacer en douceur vers l'orientation cible
        _currentPitch = MoveTowards(_currentPitch, targetPitch, _pitchSpeed * fixedDeltaTime);
        _currentRoll = MoveTowards(_currentRoll, targetRoll, _rollSpeed * fixedDeltaTime);

        // L'angle de lacet est dérivé de l'angle de roulis
        _currentYaw -= _currentRoll * _yawFromRoll * fixedDeltaTime;

        // Gestion de la vitesse basée sur le tangage vers le haut/bas
        if (_upDownInput < 0) // Piqué
        {
            float targetSpeed = _maxForwardSpeed;
            float speedLerp = 1.0f - std::exp(-_downwardSpeedSmoothing * fixedDeltaTime);
            _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
        }
        else if (_upDownInput > 0) // Montée
        {
            float targetSpeed = _minForwardSpeed;
            float speedLerp = 1.0f - std::exp(-_upwardSpeedSmoothing * fixedDeltaTime);
            _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
        }
        else // Vol horizontal
        {
            float targetSpeed = _baseForwardSpeed;
            if (_currentSpeed > targetSpeed)
                _currentSpeed = std::max(targetSpeed, _currentSpeed - _noInputDeceleration * fixedDeltaTime);
            else if (_currentSpeed < targetSpeed)
                _currentSpeed = std::min(targetSpeed, _currentSpeed + _noInputDeceleration * fixedDeltaTime);
        }
        _currentSpeed = std::clamp(_currentSpeed, _minForwardSpeed, _maxForwardSpeed);

        // Calculer la vitesse verticale (portance/chute)
        float verticalRate = -_baseSinkRate;
        if (_upDownInput < 0) // Piqué
            verticalRate = -_diveSinkRate;
        else if (_upDownInput > 0) // Montée
            verticalRate = (_currentSpeed / _maxForwardSpeed) * _climbLiftRateOffset;

        // Construire la nouvelle rotation à partir des angles de lacet, tangage et roulis
        // FIX : Ajout de JPH_PI (180 degrés) au lacet pour faire correspondre la logique physique au modèle visuel.
        Quat yawQuat = Quat::sRotation(Vec3::sAxisY(), _currentYaw + JPH::JPH_PI);
        Quat pitchQuat = Quat::sRotation(Vec3::sAxisX(), _currentPitch);
        Quat rollQuat = Quat::sRotation(Vec3::sAxisZ(), _currentRoll);
        Quat newRotation = yawQuat * pitchQuat * rollQuat;

        // Calculer la direction avant à partir de la nouvelle rotation
        Vec3 forwardDir = newRotation.RotateAxisZ();

        // Appliquer la vélocité finale
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

        // Calculer l'angle de lacet à partir du vecteur avant
        _currentYaw = std::atan2(fwd.GetX(), fwd.GetZ());

        // FIX : Compensation pour un modèle inversé
        // Si l'avion vole en arrière, cela signifie que l'avant physique est l'arrière visuel.
        // On décale de 180 degrés (PI) pour s'aligner.
        _currentYaw += JPH::JPH_PI;

        // Calculer les angles de tangage et de roulis à partir du quaternion de rotation
        Vec3 up = rot.RotateAxisY();
        _currentPitch = std::asin(-fwd.GetY());

        Vec3 right = rot.RotateAxisX();
        _currentRoll = std::atan2(right.GetY(), up.GetY());
    }

    void Plane::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        // Mettre à jour les effets visuels en fonction de la vitesse
        auto& camera = _camera.GetComponent<Camera>();
        if (auto radialBlur = camera.GetEffect<RadialBlurEffect>())
            radialBlur->SetStrength(_currentSpeed * _radialBlurSpeedFactor);
    }
} // namespace GameLogic