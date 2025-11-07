#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include "Bullet.h"
#include <Frost/Scene/Components/GameObjectInfo.h>
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
	std::vector<std::unique_ptr<Bullet>> _bullets;

	float _forward = 0.0f;
	float _previousForward = 1.0f;
	float _right = 0.0f;

	void InitializePhysics();
	void ProcessInput(float deltaTime);
	void UpdatePhysics(float deltaTime);
	void CleanupBullets();
};

