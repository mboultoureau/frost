#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>


#include "Player.h"
#include <Frost/Scene/Components/Meta.h>

using namespace Frost;

class Player;

class PlayerSpringCameraScript : public Script
{
public:
	PlayerSpringCameraScript(GameObject::Id cameraPivot, GameObject::Id thirdPersonCamera, GameObject::Id springCam, Player* player) :
		thirdPersonCamera{ thirdPersonCamera }, springCam{ springCam }, cameraPivot{ cameraPivot },
		playerManager{ player }, player{player->GetPlayerID()}, scene{ player->GetScene() }
	{
	};

	Player* playerManager;
	Scene* scene;
	GameObject::Id thirdPersonCamera;
	GameObject::Id player;
	GameObject::Id springCam;
	GameObject::Id cameraPivot;
	bool isThirdPerson = true;

	float stiffness = 7.0f;
	float defaultThirdPersonCameraDistance = -20.0f;
	float thirdPersonCamVelocityToDistance = -0.0005f;


	void OnFixedUpdate(float deltaTime) override;
	void UpdateTPCam(float deltaTime);
	void UpdateSpringCam(float deltaTime);
	void ProcessInput(float deltaTime);

private:
	float _cameraPivotRotationX = 0.0f;
	float _cameraPivotRotationY = 0.0f;
};



class PlayerCamera
{
public:
	friend class Player;

	PlayerCamera(Player* _player);

private:
	Player* _player;
	GameObject::Id _cameraPivot;
	GameObject::Id _3rdPersVirtCamera;
	GameObject::Id _camera;

	JPH::BodyID _cameraBodyID;
	JPH::BodyInterface* _bodyInter;
};