#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>


using namespace Frost;
using namespace Frost::Component;
class PlayerCamera;
class Vehicle;


class Player
{
public:
	enum VehicleType {
		BIKE = 0,
		BOAT = 1,
		PLANE = 2
	};

	Player();
	Scene* GetScene() { return _scene; };

	void SetPlayerVehicle(Player::VehicleType type);

	std::pair<VehicleType, Vehicle*> GetCurrentVehicle() { return { _currentVehicleType, _currentVehicle }; };
	int GetVehicleNumber() { return _vehicles.size(); };
	GameObject GetPlayerID() { return _playerId; };
	JPH::BodyID GetBodyID() { return _playerId.GetComponent<RigidBody>().physicBody->bodyId; };

	Timer transitionTimer;
	GameObject transitionRenderer;

private:
	GameObject _playerId;

	Scene* _scene = nullptr;
	PlayerCamera* _playerCamera = nullptr;

	Vehicle* _currentVehicle = nullptr;
	Player::VehicleType _currentVehicleType;
	std::unordered_map<Player::VehicleType, Vehicle*> _vehicles;

	void _InitializeVehicles();
	void _SummonVehicleTransition();
	void _SetBodyID(JPH::BodyID bodyId) {
		_playerId.GetComponent<RigidBody>().physicBody->bodyId = bodyId;
		auto a = _playerId.GetComponent<RigidBody>().physicBody->bodyId;
		FT_ASSERT(a == bodyId);
	}

	//void InitializePhysics();
	//void ProcessInput(float deltaTime);
	//void UpdatePhysics(float deltaTime);
};

