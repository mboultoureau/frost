#pragma once

#include "Vehicle.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace GameLogic
{
    class Boat : public Vehicle
    {
    public:
        Boat(Frost::GameObject player);
        ~Boat() override = default;

        void Show() override;
        void Hide() override;

        void OnPreFixedUpdate(float fixedDeltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        // Input handlers
        void OnMove(float right, float forward) override;
        void OnBrake(bool brake) override;
        void OnSpecialAction(bool special) override;

    private:
        void _ProcessPhysics(float fixedDeltaTime);

    private:
        Frost::GameObject _vehicle;

        // Inputs
        float _rightInput = 0.0f;
        float _forwardInput = 0.0f;
        bool _handBrakeInput = false;

        // Internal State
        float _currentForward = 0.0f;
        float _currentRight = 0.0f;
        bool _isBodyValid = false;

        // Constants (Ported from old Boat)
        const float cHalfBoatHeight = 0.4f;
        const float cHalfBoatLength = 2.0f;
        const float cHalfBoatTopWidth = 0.8f;
        const float cHalfBoatBottomWidth = 0.6f;
        const float cBoatBowLength = 0.5f;
        const float cBoatMass = 300.0f;
        const float cBoatFriction = 0.5f;

        const float cForwardAcceleration = 20.0f;
        const float cSteerAcceleration = 15.0f;
        const float cBoatBrakeStrength = 2.0f;

        // Effects constants
        const float cScreenShakeSpeedMultiplier = 0.05f;
        const float cRadialBlurSpeedFactor = 0.02f;
    };
} // namespace GameLogic