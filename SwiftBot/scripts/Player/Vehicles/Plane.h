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

        void Show() override;
        void Hide() override;

        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        void OnCollisionEnter(const Frost::Math::Vector3& contactNormal);

        void OnBrake(bool brake) override;
        void OnSpecialAction(bool special) override;
        void OnMove(float right, float forward) override;

    private:
        void _UpdateInternalStateFromBody(const JPH::Body& body, bool accountForVelocity);

        float _currentPitch = 0.0f;
        float _currentRoll = 0.0f;
        float _currentYaw = 0.0f;
        float _currentSpeed = 0.0f;

        float _leftRightInput = 0.0f;
        float _upDownInput = 0.0f;

        bool _inContinuousCollision = false;
        Frost::Timer _collisionCoolDownTimer;

        bool _justAppeared = false;

        float _noInputDeceleration = 0.4f;
        float _baseForwardSpeed = 25.0f;
        float _maxForwardSpeed = 100.0f;
        float _minForwardSpeed = 2.0f;
        float _upwardSpeedSmoothing = 1.f;
        float _downwardSpeedSmoothing = 1.0f;

        float _baseSinkRate = 2.0f;
        float _diveSinkRate = 11.0f;
        float _climbLiftRateOffset = 20.0f;

        float _pitchSpeed = 2.5f;
        float _rollSpeed = 3.0f;
        float _yawFromRoll = 1.2f;

        float _maxPitchAngle = 0.5f;
        float _maxRollAngle = 0.7f;
        float _speedAcknowledgementThreshold = 1.0f;

        float _screenShakeSpeedMultiplier = 0.08f;
        float _radialBlurSpeedFactor = 0.0005f;

        float _transferredSpeed = 0.0f;
        float _transferredForwardControl = 0.0f;

        std::chrono::milliseconds _collisionCoolDownDuration = 750ms;
    };
} // namespace GameLogic