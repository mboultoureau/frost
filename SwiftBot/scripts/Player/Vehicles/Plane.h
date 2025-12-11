#pragma once

#include "Frost.h"
#include "Player/Vehicles/Vehicle.h"

#include <Jolt/Jolt.h>

namespace GameLogic
{
    class Plane : public Vehicle
    {
    public:
        Plane(Frost::GameObject player);

        // Overrides du cycle de vie
        void Show() override;
        void Hide() override;

        // Overrides des mises à jour
        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        // Gestionnaires de collision
        void OnCollisionEnter(const Frost::Math::Vector3& contactNormal);

        // Overrides des entrées
        void OnBrake(bool brake) override;
        void OnSpecialAction(bool special) override;
        void OnMove(float right, float forward) override;

    private:
        void _UpdateInternalStateFromBody(bool accountForVelocity);

        // État
        float _currentPitch = 0.0f;
        float _currentRoll = 0.0f;
        float _currentYaw = 0.0f;
        float _currentSpeed = 0.0f;

        // Entrées
        float _leftRightInput = 0.0f;
        float _upDownInput = 0.0f;

        // Collision
        bool _inContinuousCollision = false;
        Frost::Timer _collisionCoolDownTimer;

        // Logique d'apparition
        bool _justAppeared = false;

        // --- Paramètres du modèle de vol ---

        // Vitesse
        float _noInputDeceleration = 4.0f; // Anciennement 0.4f
        float _baseForwardSpeed = 250.0f;  // Anciennement 25.0f
        float _maxForwardSpeed = 500.0f;   // Anciennement 50.0f
        float _minForwardSpeed = 20.0f;    // Anciennement 2.0f
        float _upwardSpeedSmoothing = 0.5f;
        float _downwardSpeedSmoothing = 1.0f;

        // Portance/Chute
        float _baseSinkRate = 2.0f;          // Anciennement 2.0f
        float _diveSinkRate = 55.0f;         // Anciennement 11.0f
        float _climbLiftRateOffset = 200.0f; // Anciennement 20.0f

        // Rotation
        float _pitchSpeed = 12.0f; // Anciennement 2.5f
        float _rollSpeed = 15.0f;  // Anciennement 3.0f
        float _yawFromRoll = 6.0f; // Anciennement 1.2f

        // Limites
        float _maxPitchAngle = 0.5f; // ~30 degrés
        float _maxRollAngle = 0.7f;  // ~40 degrés
        float _speedAcknowledgementThreshold = 1.0f;

        // Effets
        float _screenShakeSpeedMultiplier = 0.08f;
        float _radialBlurSpeedFactor = 0.001f;

        // Temps de recharge de la collision
        std::chrono::milliseconds _collisionCoolDownDuration = 750ms;
    };
} // namespace GameLogic