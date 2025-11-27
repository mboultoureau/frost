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
    float currentPitch = 0.0f;
    float currentRoll = 0.0f;
    float currentYaw = 0.0f;
    float currentSpeed = 0.0f;

    float leftRightInput = 0;
    float upDownInput = 0;

    bool inContinuousCollision = false;

    float noInputDeceleration = 0.5f;

    float baseForwardSpeed = 25.0f;
    float maxForwardSpeed = 55.0f;
    float minForwardSpeed = 2.0f;
    float speedSmoothing = 1.0f;

    float baseSinkRate = 2.0f;
    float diveSinkRate = 8.0f;
    float climbLiftRate = 1.5f;

    float pitchSpeed = 2.5f;
    float rollSpeed = 3.0f;
    float yawFromRoll = 1.2f;

    float maxPitchAngle = 0.5f; // ~30 degres
    float maxRollAngle = 0.7f;  // ~40 degres

    std::chrono::milliseconds collisionCoolDown = 750ms; // seconds
    Timer collisionCoolDownTimer;

    float MoveTowards(float current, float target, float maxDelta)
    {
        float diff = target - current;
        if (std::abs(diff) <= maxDelta)
        {
            return target;
        }
        return current + std::copysign(maxDelta, diff);
    }
};