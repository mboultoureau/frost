#pragma once

#include "Frost.h"
#include "Player/Vehicles/Vehicle.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>

namespace GameLogic
{
    class Moto : public Vehicle
    {
    public:
        Moto(Frost::GameObject player);

        void Show() override;
        void Hide() override;

        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        virtual void OnBrake(bool brake) override;
        virtual void OnSpecialAction(bool special) override;
        virtual void OnMove(float right, float forward) override;

    private:
        void _ProcessBikeInput(float fixedDeltaTime);
        void _GiveBoost();

    private:
        JPH::VehicleConstraint* _constraint = nullptr;
        JPH::MotorcycleController* _controller = nullptr;

        JPH::Vec3 _previousLinearSpeed;
        JPH::Vec3 _previousAngularSpeed;

        // Drift / Boost vars
        Frost::Timer _specialDriftTimer;
        Frost::Timer _specialDriftCoolDown;

        std::chrono::milliseconds _driftCoolDownDuration = 100ms;
        float _specialDriftMaxDuration = 1000.0f;
        float _specialDriftPower = 250.0f;
        float _speedAtDriftStart = 0.0f;

        float _maxSpeed = 60.0f;

        // Controls
        float _forward = 0;
        float _right = 0;
        float _previousForward = 0;
        float _brake = 0;

        // Input
        float _rightInput = 0;
        float _forwardInput = 0;
        bool _handBrakeInput = false;
        bool _specialInput = false;

        // Screen Shake
        float _screenShakeDuration = 0.2f;
        float _screenShakeSpeedMultiplier = 0.015f;
        float _shakeLinearSpeedDiffThreshold = 0.5f;
        float _shakeAngularSpeedDiffThreshold = 0.6f;
        float _shakeSpeedAngleDiffThreshold = 0.6f;

        float _transferredSpeed = 0.0f;
        float _transferredForwardControl = 0.0f;

        // Radial Blur
        float _radialBlurSpeedFactor = 0.001f;

        // Wheels
        Frost::GameObject _frontWheel;
        Frost::GameObject _backWheel;
    };
} // namespace GameLogic