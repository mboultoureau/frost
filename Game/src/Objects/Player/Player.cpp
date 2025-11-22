#include "Player.h"
#include "../../Physics/PhysicsLayer.h"
#include "../../Game.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "Frost/Scene/Components/Meta.h"
#include "PlayerCamera.h"

#include "../CheckPoint.h"
#include "Vehicles/Bike.h"
#include "Vehicles/Plane.h"
#include "PlayerScript.h"
#include "Vehicles/Vehicle.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Player::Player()
{
	_scene = &Game::GetScene();

	// Create Player Game Object -------------
	_playerId = _scene->CreateGameObject("Player");
	_playerId.AddComponent<Transform>(
		Vector3{ -365.0f, 68.5f, -100.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0f },
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_playerId.AddComponent<WorldTransform>();

	// Create TransitionModelRenderer -----------
	transitionRenderer = _scene->CreateGameObject("Transition Model renderer", _playerId);
	transitionRenderer.AddComponent<Transform>(
		Vector3{ 0.0f, 0, 0.0f },
		EulerAngles{ 0.0f, 0.0f, 0.0f },
		Vector3{ 2.0f, 2.0f, 2.0f });
	transitionRenderer.AddComponent<WorldTransform>();
	transitionRenderer.AddComponent<StaticMesh>("./resources/meshes/sphere.fbx");

	// Create Vehicules structures -------------
	_InitializeVehicles();
	SetPlayerVehicle(VehicleType::BIKE);
	_playerId.AddScript<PlayerScript>(this);


	// Create playerCameras Game Objects -------------
	auto pCam = PlayerCamera(this);
	_playerCamera = &pCam;
}


void Player::_InitializeVehicles()
{
	auto pos = Vector3{ 0.0f, 0.0f, 0.0f };
	// Bike
	auto bike = new Bike(
		this, 
		Vehicle::RendererParameters(
			"Moto Renderer",
			"./resources/meshes/moto.glb",
			Vector3{0,-0.4f,0},
			EulerAngles{ 0.0, 0.0f, -90.0_deg },
			Vector3{ .6f, .6f, .6f }
		));
	_vehicles.insert({ VehicleType::BIKE, bike });

	// Boat
	auto boat = new Bike(
		this,
		Vehicle::RendererParameters(
			"Boat Renderer",
			"./resources/meshes/pill.fbx",
			pos,
			EulerAngles{ -90.0_deg, 0.0f, 0.0f },
			Vector3{ .6f, .6f, .6f }
		));
	_vehicles.insert({ VehicleType::BOAT, boat });



	// Plane
	auto plane = new Plane(
		this,
		Vehicle::RendererParameters(
			"Plane Renderer",
			"./resources/meshes/paper_airplane.glb",
			pos,
			EulerAngles{ 0.0_deg, 180.0_deg, 0.0f },
			Vector3{ 10.0f, 10.0f, 10.0f }
		));
	_vehicles.insert({ VehicleType::PLANE, plane });

}

void Player::SetPlayerVehicle(Player::VehicleType type)
{
	using namespace JPH;
	Vec3 linearSpeed;
	Vec3 angularSpeed;
	
	if (_currentVehicle)
	{
		Physics::Get().body_interface->GetLinearAndAngularVelocity(
			_playerId.GetComponent<RigidBody>().physicBody->bodyId,
			linearSpeed,
			angularSpeed
		);
		_currentVehicle->Disappear();
	}
	_currentVehicle = _vehicles[type];
	_currentVehicleType = type;
	_SummonVehicleTransition();
	auto bodyId = _currentVehicle->Appear();
	if (_playerId.HasComponent<RigidBody>())
	{
		_SetBodyID(bodyId);
		Physics::Get().body_interface->SetLinearAndAngularVelocity(
			_playerId.GetComponent<RigidBody>().physicBody->bodyId,
			linearSpeed,
			angularSpeed
		);
	}
	else
	{
		_playerId.AddComponent<RigidBody>(_currentVehicle->GetBodyID(), _playerId);
	}
}
void Player::_SummonVehicleTransition()
{
	FT_INFO("youhou, you have changed vehicle !");
	transitionRenderer.SetActive(true);
	transitionTimer.Resume();
	transitionTimer.Start();
}


