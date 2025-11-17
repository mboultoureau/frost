#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include <Frost/Scene/Components/Meta.h>

using namespace Frost;



class PlayerSpringCameraScript : public Script
{
public:
	PlayerSpringCameraScript(Scene* scene, GameObject::Id cameraPivot, GameObject::Id thirdPersonCamera, GameObject::Id player, GameObject::Id vehicle, GameObject::Id springCam) :
		scene{ scene }, thirdPersonCamera{ thirdPersonCamera }, springCam{ springCam },
		player{ player },
		vehicle{vehicle},
		cameraPivot{ cameraPivot }
	{
	};

	Scene* scene;
	GameObject::Id thirdPersonCamera;
	GameObject::Id player;
	GameObject::Id vehicle;
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

	PlayerCamera(GameObject::Id& _player, GameObject::Id& _vehicle);

private:
	GameObject::Id _player;
	GameObject::Id _cameraPivot;
	GameObject::Id _3rdPersVirtCamera;
	GameObject::Id _camera;

	JPH::BodyID _cameraBodyID;
	JPH::BodyInterface* _bodyInter;
};