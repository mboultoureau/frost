#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include <Frost/Scene/Components/Meta.h>
#include "PlayerCamera.h"

using namespace Frost;

class Player
{
public:
	Player();
	void FixedUpdate(float deltaTime);

private:
	GameObject::Id _player;
	GameObject::Id _vehicle;
	PlayerCamera* _playerCamera;

	JPH::BodyID _playerBodyID;
	JPH::BodyID _cameraBodyID;
	JPH::BodyInterface* _bodyInter;

	Timer _fireTimer;

	float _forward = 0.0f;
	float _previousForward = 1.0f;
	float _right = 0.0f;

	void InitializePhysics();
	void ProcessInput(float deltaTime);
	void UpdatePhysics(float deltaTime);
};

