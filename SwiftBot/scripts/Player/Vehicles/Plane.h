#pragma once

#include "Vehicle.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Frost/Core/Timer.h>
#include <numbers>
#include <cmath>
#include <chrono>

namespace GameLogic
{
    // Fonction helper hors classe pour les constantes statiques
    constexpr float DegreesToRadians(float deg)
    {
        return deg * (std::numbers::pi_v<float> / 180.0f);
    }

    class Plane : public Vehicle
    {
    public:
        Plane(Frost::GameObject player);
        ~Plane() override = default;

        void Show() override;
        void Hide() override;

        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        // Gestion des collisions
        void OnCollisionEnter(const Frost::Math::Vector3& contactNormal);

        void OnMove(float right, float forward) override;
        void OnBrake(bool brake) override;
        void OnSpecialAction(bool special) override;

    private:
        void _UpdateInternalStateFromBody(bool accountForVelocity);

    private:
        Frost::GameObject _vehicle;

        // Inputs
        float _leftRightInput = 0.0f;
        float _upDownInput = 0.0f;

        // Flight State
        float _currentSpeed = 0.0f;
        float _currentPitch = 0.0f;
        float _currentRoll = 0.0f;
        float _currentYaw = 0.0f;

        bool _justAppeared = false;
        bool _inContinuousCollision = false;

        // Timer et Cooldowns
        Frost::Timer _collisionCoolDownTimer;
        static constexpr auto _collisionCoolDownDuration = std::chrono::milliseconds(750);
        static constexpr float _speedAcknowledgementThreshold = 1.0f;

        // Settings / Constants
        static constexpr float _baseForwardSpeed = 30.0f;
        static constexpr float _minForwardSpeed = 15.0f;
        static constexpr float _maxForwardSpeed = 80.0f;

        static constexpr float _maxPitchAngle = DegreesToRadians(60.0f);
        static constexpr float _maxRollAngle = DegreesToRadians(80.0f);

        static constexpr float _pitchSpeed = 2.0f;
        static constexpr float _rollSpeed = 2.5f;
        static constexpr float _yawFromRoll = 0.5f;

        static constexpr float _downwardSpeedSmoothing = 2.0f;
        static constexpr float _upwardSpeedSmoothing = 0.5f;
        static constexpr float _noInputDeceleration = 5.0f;

        static constexpr float _baseSinkRate = 0.0f;
        static constexpr float _diveSinkRate = 5.0f;
        static constexpr float _climbLiftRateOffset = 10.0f;

        static constexpr float _radialBlurSpeedFactor = 0.01f;
        static constexpr float _screenShakeSpeedMultiplier = 0.005f;
    };
} // namespace GameLogic