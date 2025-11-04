#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include "Bullet.h"

using namespace Frost;

class Player
{
public:
	Player();
	void FixedUpdate(float deltaTime);

private:
	GameObject::Id _player;
	GameObject::Id _cameraPivot;
	GameObject::Id _camera;
	GameObject::Id _vehicle;

	JPH::BodyID _playerBodyID;
	JPH::BodyInterface* _bodyInter;

	Timer _fireTimer;
	std::vector<std::unique_ptr<Bullet>> _bullets;

	float _forward = 0.0f;
	float _previousForward = 1.0f;
	float _right = 0.0f;
	float _cameraPivotRotationX = 0.0f;
	float _cameraPivotRotationY = 0.0f;

	void InitializePhysics();
	void ProcessInput(float deltaTime);
	void UpdatePhysics(float deltaTime);
	void CleanupBullets();
};

