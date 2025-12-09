#pragma once

#include "Frost.h"
#include "Player/Vehicles/Vehicle.h"

#include <Jolt/Jolt.h>

namespace GameLogic
{
    class Moto : public Vehicle
    {
    public:
        Moto(Frost::GameObject player, Frost::GameObject vehicle);

        void Show() override;
        void Hide() override;

    private:
    };

    /*
    class Bike : public Vehicle
    {
    public:

        Bike(Player* player, RendererParameters params);

        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;
        void OnLateUpdate(float deltaTime) override;

        void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;
        void OnCollisionStay(BodyOnContactParameters params, float deltaTime) override;
        // Warning : params may contains bodies that are not valid at the moment
        void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) override;

        void OnLeftRightInput(float deltaTime, float leftRightInput) override;
        void OnAccelerateInput(float deltaTime, float upDownInput) override;
        void OnBrake(float deltaTime, bool handBrakeInput) override;
        void GiveBoost();
        void OnSpecial(float deltaTime, bool specialInput) override;

        JPH::BodyID Appear() override;
        void Disappear() override;

        void ProcessBikeInput(float deltaTime);

    private:
        JPH::VehicleConstraint* mConstraint = nullptr;
        JPH::MotorcycleController* mController = nullptr;
        JPH::Body* _body;
        JPH::Vec3 _previousLinearSpeed;
        JPH::Vec3 _previousAngularSpeed;
        Vehicle* _vehicle;

        Timer _specialDriftTimer;
        Timer _specialDriftCoolDown;
        std::chrono::milliseconds _driftCoolDownDuration = 100ms;

        float _specialDriftMaxDuration = 100;
        float _specialDriftPower = 250;
        float _specialDriftRotationPower = -20;
        float _speedAtDriftStart = 0;

        float _maxSpeed = 30;

        float _forward = 0;
        float _right = 0;
        float _previousForward = 0;
        float _brake = 0;

        float _leftRightInput = 0;
        float _upDownInput = 0;
        bool _handBrakeInput = false;
        bool _specialInput = false;

        float _screenShakeDuration = 0.2f;
        float _screenShakeSpeedMultiplier = 0.015f;
        float _shakeLinearSpeedDiffThreshold = 0.5f;
        float _shakeAngularSpeedDiffThreshold = 0.6f;
        float _shakeSpeedAngleDiffThreshold = 0.6f;

        float _radialBlurSpeedFactor = 0.001f;
    };
*/
} // namespace GameLogic
