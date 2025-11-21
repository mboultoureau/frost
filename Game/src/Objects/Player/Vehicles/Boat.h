#pragma once

#include "Frost.h"
#include "Vehicle.h"
#include "../Player.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>



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
	* WARNING : 'OnCollisionExit' cannot guarantee that the bodies involved in collision still exist.
	* Always verify that the rigidbodies are valid
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
	//void OnSpecial(float deltaTime) override;

	JPH::BodyID Appear() override;
	void Disappear() override;

	void ProcessBikeInput(float deltaTime);

private:
	JPH::VehicleConstraint* mConstraint = nullptr;
	JPH::MotorcycleController* mController = nullptr;
	JPH::Body* _body;

	float _forward = 0;
	float _right = 0;
	float _previousForward = 0;
	float _brake = 0;

	float _leftRightInput;
	float _upDownInput;
	bool _handBrakeInput = false;
};