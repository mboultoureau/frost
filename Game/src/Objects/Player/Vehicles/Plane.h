#pragma once

#include "../Player.h"
#include "Frost.h"
#include "Vehicle.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

using namespace Frost;

class Plane : public Vehicle
{
public:
    Plane(Player* player, RendererParameters params);
    void OnPreFixedUpdate(float fixedDeltaTime) override;
    void OnFixedUpdate(float fixedDeltaTime) override;
    void OnLateUpdate(float deltaTime) override;

    void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;
    // void OnCollisionStay(BodyOnContactParameters params, float deltaTime)
    // override;
    //  Warning : params may contains bodies that are not valid at the moment
    void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) override;

    void OnLeftRightInput(float deltaTime, float leftRightInput) override;
    void OnAccelerateInput(float deltaTime, float upDownInput) override;
    void OnBrake(float deltaTime, bool handBrakeInput) override;

    JPH::BodyID Appear() override;
    void Disappear() override;

private:
    float _currentPitch = 0.0f;
    float _currentRoll = 0.0f;
    float _currentYaw = 0.0f;
    float _currentSpeed = 0.0f;

    float _leftRightInput = 0;
    float _upDownInput = 0;

    bool _inContinuousCollision = false;
    bool _justAppeared = false;

    float _noInputDeceleration = 0.4f;

    float _baseForwardSpeed = 25.0f;
    float _maxForwardSpeed = 50.0f;
    float _minForwardSpeed = 2.0f;
    float _upwardSpeedSmoothing = 0.5f;
    float _downwardSpeedSmoothing = 1.0f;

    float _baseSinkRate = 2.0f;
    float _diveSinkRate = 11.0f;
    float _climbLiftRateOffset = 20.0f;

    float _pitchSpeed = 2.5f;
    float _rollSpeed = 3.0f;
    float _yawFromRoll = 1.2f;

    float _maxPitchAngle = 0.5f; // ~30 degres
    float _maxRollAngle = 0.7f;  // ~40 degres
    float _speedAcknowledgementThreshold = 1.0f;

    float _screenShakeDuration = 0.4f;
    float _screenShakeSpeedMultiplier = 0.08f;

    float _radialBlurSpeedFactor = 0.001f;

    std::chrono::milliseconds _collisionCoolDown = 750ms;
    Timer _collisionCoolDownTimer;

    float MoveTowards(float current, float target, float maxDelta)
    {
        float diff = target - current;
        if (std::abs(diff) <= maxDelta)
        {
            return target;
        }
        return current + std::copysign(maxDelta, diff);
    }

    void UpdateInternalStateFromBody(bool accountForVelocity)
    {
        auto& bodyInterface = Physics::GetBodyInterface();

        JPH::Quat rot = bodyInterface.GetRotation(_bodyId);
        JPH::Vec3 speed = bodyInterface.GetLinearVelocity(_bodyId);

        JPH::Vec3 fwd;

        if (accountForVelocity && speed.Length() > _speedAcknowledgementThreshold)
        {
            fwd = speed.NormalizedOr(rot.RotateAxisZ());
        }
        else
        {
            fwd = rot.RotateAxisZ();
        }

        _currentYaw = std::atan2(fwd.GetX(), fwd.GetZ());

        JPH::Vec3 up = rot.RotateAxisY();
        _currentPitch = std::asin(-fwd.GetY());

        JPH::Vec3 right = rot.RotateAxisX();
        _currentRoll = std::atan2(right.GetY(), up.GetY());
    }
};