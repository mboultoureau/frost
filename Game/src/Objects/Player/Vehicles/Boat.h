#pragma once

#include "../Player.h"
#include "Frost.h"
#include "Vehicle.h"

#include "../../Water.h"
#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

using namespace Frost;

class Boat : public Vehicle
{
public:
    /*
     * ======== Event orders =============
     * - Fixed Update
     * - Inputs
     *	-> Brake
     *	-> Left / Right
     *	-> Accelerate
     * - Late Update
     *
     * Collision events are independent and might be called at any time
     * WARNING : 'OnCollisionExit' cannot guarantee that the bodies involved in
     *collision still exist. Always verify that the rigidbodies are valid
     */

    Boat(Player* player, RendererParameters params);

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
    // void OnSpecial(float deltaTime) override;

    JPH::BodyID Appear() override;
    void Disappear() override;

    void ProcessBoatInput(float deltaTime);

    void SetBoatWater(Water* w) { _currentWater = w; };

    static bool IsBoatID(JPH::BodyID id) { return boatIds.find(id) != boatIds.end(); }
    static void RegisterBoat(JPH::BodyID id, Boat* b) { boatIds.insert({ id, b }); };
    static void UnregisterBoat(JPH::BodyID id) { boatIds.erase(id); };
    static void SetBoatWater(JPH::BodyID id, Water* w) { boatIds[id]->SetBoatWater(w); };

private:
    static std::unordered_map<JPH::BodyID, Boat*> boatIds;

    Water* _currentWater = nullptr;

    float frictionFactor = 0.25f;

    float _forward = 0;
    float _right = 0;
    float _previousForward = 0;
    float _brake = 0;

    float _leftRightInput;
    float _upDownInput;
    bool _handBrakeInput = false;

    static constexpr float cBoatMass = 1000.0f;
    static constexpr float cBoatFriction = 30.0f;

    static constexpr float cHalfBoatLength = 3.75f;
    static constexpr float cHalfBoatTopWidth = .4f;
    static constexpr float cHalfBoatBottomWidth = .25f;
    static constexpr float cBoatBowLength = 0.5f; // 3.5f;
    static constexpr float cHalfBoatHeight = 0.2f;

    static constexpr float cForwardAcceleration = 30.0f;
    static constexpr float cSteerAcceleration = 3.0f;
};