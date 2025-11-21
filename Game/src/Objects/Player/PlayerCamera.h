#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>


#include "Player.h"
#include <Frost/Scene/Components/Meta.h>
#include "Frost/Scene/Components/Script.h"

using namespace Frost;

class Player;

class PlayerSpringCameraScript : public Script
{
public:
	PlayerSpringCameraScript(GameObject cameraPivot, GameObject thirdPersonCamera, GameObject springCam, Player* player) :
		thirdPersonCamera{ thirdPersonCamera }, springCam{ springCam }, cameraPivot{ cameraPivot },
		playerManager{ player }, player{player->GetPlayerID()}, scene{ player->GetScene() }
	{
	};

	Player* playerManager;
	Scene* scene;
	GameObject thirdPersonCamera;
	GameObject player;
	GameObject springCam;
	GameObject cameraPivot;
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
	GameObject _cameraPivot;
	GameObject _3rdPersVirtCamera;
	GameObject _camera;

	JPH::BodyID _cameraBodyID;
	JPH::BodyInterface* _bodyInter;
};