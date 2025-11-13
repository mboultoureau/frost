#include "PlayerCamera.h"

#include "Frost.h"
#include "Player.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>     

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include <Frost/Scene/Components/GameObjectInfo.h>


void PlayerSpringCameraScript::OnFixedUpdate(float deltaTime)
{
	ProcessInput(deltaTime);
	UpdateTPCam(deltaTime);
	UpdateSpringCam(deltaTime);
}

void PlayerSpringCameraScript::UpdateTPCam(float deltaTime)
{
	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(
		_cameraPivotRotationX,
		_cameraPivotRotationY,
		0.0f
	);

	auto cameraPivotTransform = scene->GetComponent<Transform>(cameraPivot);
	DirectX::XMStoreFloat4(&cameraPivotTransform->rotation, quaternion);

	// TODO : cam rotation is actually one frame behind cameraPivot rotation
	auto cameraPivotWTransform = scene->GetComponent<WorldTransform>(cameraPivot);

	auto springCamRB = scene->GetComponent<RigidBody>(springCam);

	// set desired pos to 3rd pers cam
	if (isThirdPerson) { 
		auto thirdPersonCameraWTransform = scene->GetComponent<WorldTransform>(thirdPersonCamera);
		Physics::Get().body_interface->SetRotation(springCamRB->physicBody->bodyId,
			JPH::Quat(thirdPersonCameraWTransform->rotation.x,
				thirdPersonCameraWTransform->rotation.y,
				thirdPersonCameraWTransform->rotation.z,
				thirdPersonCameraWTransform->rotation.w),
			JPH::EActivation::Activate);
	}//Set desired pos to player
	else { 
		auto playerWTransform = scene->GetComponent<WorldTransform>(player);
		Physics::Get().body_interface->SetRotation(springCamRB->physicBody->bodyId,
			JPH::Quat(playerWTransform->rotation.x,
				playerWTransform->rotation.y,
				playerWTransform->rotation.z,
				playerWTransform->rotation.w),
			JPH::EActivation::Activate);
	}


	//Set camera pos according to player velocity
	auto playerRB = scene->GetComponent<RigidBody>(player);
	auto thirdPersonCameraTransform = scene->GetComponent<Transform>(thirdPersonCamera);

	auto velocity = Physics::Get().body_interface->GetLinearVelocity(playerRB->physicBody->bodyId);
	auto speed = velocity.LengthSq();
	thirdPersonCameraTransform->position.z = defaultThirdPersonCameraDistance + speed * thirdPersonCamVelocityToDistance;
}

class IgnoreCameraLayerFilter : public JPH::ObjectLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer inLayer) const override
	{
		return inLayer != ObjectLayers::CAMERA && inLayer != ObjectLayers::PLAYER && inLayer != ObjectLayers::CHECKPOINT;   // ignore la camera
	}
};

class RayCastBroadPhaseFilter : public JPH::BroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
	{
		return true;
	}
};

void PlayerSpringCameraScript::UpdateSpringCam(float deltaTime) {
	
	auto playerWTransform = scene->GetComponent<WorldTransform>(player);
	auto thirdPersonCameraWTransform = scene->GetComponent<WorldTransform>(thirdPersonCamera);
	auto springCamRigidBody = scene->GetComponent<RigidBody>(springCam);

	JPH::Vec3 cameraPos = Physics::Get().body_interface->GetPosition(springCamRigidBody->physicBody->bodyId);
	JPH::Vec3 cameraVelocity = Physics::Get().body_interface->GetLinearVelocity(springCamRigidBody->physicBody->bodyId);
	auto desiredPos = isThirdPerson ?
	Physics::Vector3ToJoltVector(thirdPersonCameraWTransform->position) :
	Physics::Vector3ToJoltVector(playerWTransform->position);

	auto playerPosition = Physics::Vector3ToJoltVector(playerWTransform->position);
	JPH::RRayCast ray;
	ray.mOrigin = playerPosition;
	auto _thirdPCamPos = Physics::Vector3ToJoltVector(thirdPersonCameraWTransform->position);
	ray.mDirection = (_thirdPCamPos - playerPosition);
	float desiredDistance = ray.mDirection.Length();

	auto renderer = scene->GetComponent<ModelRenderer>(vehicle);
	renderer->isActive = ((playerPosition - cameraPos).Length() > 10.f);

	if (isThirdPerson) {
		RayCastBroadPhaseFilter bpFilter;
		IgnoreCameraLayerFilter objectFilter;

		JPH::RayCastResult result;
		if (desiredDistance > 0.01 && Physics::Get().physics_system.GetNarrowPhaseQuery().CastRay(ray, result, bpFilter, objectFilter))
		{
			cameraPos = ray.GetPointOnRay(result.mFraction);
			Physics::Get().body_interface->SetPosition(springCamRigidBody->physicBody->bodyId, cameraPos, JPH::EActivation::Activate);
			return;
		}
	}

	JPH::Vec3 displacement = desiredPos - cameraPos;
	JPH::Vec3 springForce = displacement * stiffness;
	Physics::Get().body_interface->SetPosition(springCamRigidBody->physicBody->bodyId, cameraPos + deltaTime * springForce, JPH::EActivation::Activate);
}




void PlayerSpringCameraScript::ProcessInput(float deltaTime) {
	// Rotate Y
	if (Input::GetKeyboard().IsKeyDown(K_1) || Input::GetKeyboard().IsKeyDown(K_NUMPAD6))
	{
		_cameraPivotRotationY += 1.0f * deltaTime;
	}
	else if (Input::GetKeyboard().IsKeyDown(K_2) || Input::GetKeyboard().IsKeyDown(K_NUMPAD4))
	{
		_cameraPivotRotationY -= 1.0f * deltaTime;
	}

	// Rotate X
	if (Input::GetKeyboard().IsKeyDown(K_3) || Input::GetKeyboard().IsKeyDown(K_NUMPAD8))
	{
		_cameraPivotRotationX += 1.0f * deltaTime;
	}
	else if (Input::GetKeyboard().IsKeyDown(K_4) || Input::GetKeyboard().IsKeyDown(K_NUMPAD2))
	{
		_cameraPivotRotationX -= 1.0f * deltaTime;
	}

	// Switch cam to 3rd person or 1st person
	if (Input::GetKeyboard().GetKeyState(K_E) == KeyState::DOWN)
	{
		isThirdPerson = !isThirdPerson;
	}
	isThirdPerson = !(Input::GetKeyboard().IsKeyDown(K_E));
}


PlayerCamera::PlayerCamera(GameObject::Id& _player, GameObject::Id& _vehicle) : _player{_player}
{
	using namespace JPH;
	auto& scene = Game::GetScene();

	// Camera Pivot
	_cameraPivot = scene.CreateGameObject("Camera Pivot", _player);
	scene.AddComponent<Transform>(_cameraPivot, Transform::Vector3{ 0.0f, 0.0f, 4.4f });
	scene.AddComponent<WorldTransform>(_cameraPivot);


	// 3rdPersVirtualCamera
	_3rdPersVirtCamera = scene.CreateGameObject("3rd Person Virtual Camera", _cameraPivot);
	scene.AddComponent<Transform>(_3rdPersVirtCamera, Transform::Vector3{ 0.0f, 10, -20.0f });
	scene.AddComponent<WorldTransform>(_3rdPersVirtCamera, Transform::Vector3{ 0.0f, 10.0f, -20.0f });
	//debug
		//scene.AddComponent<ModelRenderer>(_3rdPersVirtCamera, "./resources/meshes/sphere.fbx");
	auto tpCamWTransform = scene.GetComponent<WorldTransform>(_3rdPersVirtCamera);

	// Camera
	_camera = scene.CreateGameObject("Camera");
	scene.AddComponent<Transform>(_camera, Transform::Vector3{ 0.0f, 10, -20.0f });
	scene.AddComponent<WorldTransform>(_camera, Transform::Vector3{ 0.0f, 10.0f, -20.0f });
	scene.AddComponent<Camera>(_camera);
	scene.AddComponent<ModelRenderer>(_camera, "./resources/meshes/sphere.fbx");

	auto cameraComponent = scene.GetComponent<Camera>(_camera);
	cameraComponent->backgroundColor[0] = 47.0f / 255.0f;
	cameraComponent->backgroundColor[1] = 116.0f / 255.0f;
	cameraComponent->backgroundColor[2] = 228.0f / 255.0f;
	cameraComponent->backgroundColor[3] = 1.0f;

	// Create the Camera Sensor
	JPH::ShapeRefC sphereShape = JPH::SphereShapeSettings(1.0f).Create().Get();
	BodyCreationSettings camera_body_settings(sphereShape, 
		Physics::Vector3ToJoltVector(scene.GetComponent<Transform>(_player)->position),
		Quat::sIdentity(), 
		EMotionType::Dynamic, 
		ObjectLayers::CAMERA
	);
	camera_body_settings.mGravityFactor = 0.0f;
	camera_body_settings.mIsSensor = true;
	scene.AddComponent<RigidBody>(_camera, camera_body_settings, _camera, EActivation::Activate);
	scene.AddScript<PlayerSpringCameraScript>(
		_camera, 
		&scene,
		_cameraPivot,
		_3rdPersVirtCamera,
		_player,
		_vehicle,
		_camera
		);
		
	_cameraBodyID = scene.GetComponent<RigidBody>(_camera)->physicBody->bodyId;
	_bodyInter = Physics::Get().body_interface;
}