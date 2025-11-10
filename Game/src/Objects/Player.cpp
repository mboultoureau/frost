#include "Player.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>


// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include <Frost/Scene/Components/GameObjectInfo.h>
#include "PlayerCamera.h"

#include "CheckPoint.h"

Player::Player()
{
	Scene& scene = Game::GetScene();

	_player = scene.CreateGameObject("Player");
	scene.AddComponent<Transform>(
		_player,
		Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	scene.AddComponent<WorldTransform>(_player);

	// Vehicle Model
	_vehicle = scene.CreateGameObject("Vehicle", _player);
	scene.AddComponent<Transform>(
		_vehicle,
		Frost::Transform::Vector3{ 0.0f, 1.0f, 5.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f }, //-90,0,0
		Frost::Transform::Vector3{ 5.0f, 5.0f, 5.0f });

	scene.AddComponent<WorldTransform>(_vehicle);
	scene.AddComponent<ModelRenderer>(_vehicle, "./resources/meshes/sphere.fbx"); //"./resources/meshes/moto.glb"

	//Add spring camera
	auto wt = scene.GetComponent<WorldTransform>(_player);
	auto pCam = PlayerCamera(_player, _vehicle);
	_playerCamera = &pCam;
	
	InitializePhysics();
	
	_fireTimer.Start();
}

void Player::FixedUpdate(float deltaTime)
{
	ProcessInput(deltaTime);
	UpdatePhysics(deltaTime);
}

void Player::InitializePhysics()
{
	using namespace JPH;

	FT_ENGINE_ASSERT(_player != GameObject::InvalidId, "Player GameObject is invalid");
	Scene& scene = Game::GetScene();

	// Create vehicle body
	RVec3 position(-365, 100.0f, -100);
	JPH::ShapeRefC boxShape = JPH::SphereShapeSettings(5.0f).Create().Get();	//5 5 25
	BodyCreationSettings motorcycle_body_settings(boxShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
	motorcycle_body_settings.mRestitution = 0.0f;
	motorcycle_body_settings.mAllowSleeping = false;
	motorcycle_body_settings.mFriction = 100.f; 
	motorcycle_body_settings.mAllowedDOFs = 
		EAllowedDOFs::RotationY | EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
	scene.AddComponent<RigidBody>(_player, motorcycle_body_settings, _player, EActivation::Activate);

	_playerBodyID = scene.GetComponent<RigidBody>(_player)->bodyId;
	_cameraBodyID = scene.GetComponent<RigidBody>(_playerCamera->_camera)->bodyId;
	_bodyInter = Physics::Get().body_interface;

	_bodyInter->SetMotionQuality(_playerBodyID, EMotionQuality::Discrete);
}

void Player::ProcessInput(float deltaTime)
{
	auto& scene = Game::GetScene();

	_forward = 0.0f;
	_right = 0.0f;
	if (Input::GetKeyboard().IsKeyDown(K_UP) || Input::GetKeyboard().IsKeyDown(K_W))
	{
		_forward = 1.0f;
	}
	else if (Input::GetKeyboard().IsKeyDown(K_DOWN) || Input::GetKeyboard().IsKeyDown(K_S))
	{
		_forward -= 1.0f;
	}

	// Steering
	float right = 0.0f;
	if (Input::GetKeyboard().IsKeyDown(K_LEFT) || Input::GetKeyboard().IsKeyDown(K_A))
	{
		_right = -1.0f * deltaTime * 50.0f;
	}
	else if (Input::GetKeyboard().IsKeyDown(K_RIGHT) || Input::GetKeyboard().IsKeyDown(K_D))
	{
		_right = 1.0f * deltaTime * 50.0f;
	}

}



void Player::UpdatePhysics(float deltaTime)
{
	using namespace JPH;

	auto pos = _bodyInter->GetPosition(_playerBodyID);
	if (pos.GetY() < 60) {
		Scene& scene = Game::GetScene();
		auto cpTransform = scene.GetComponent<Transform>(CheckPoint::lastCheckPoint);
		_bodyInter->SetPosition(_playerBodyID, Physics::Vector3ToJoltVector(cpTransform->position), EActivation::Activate);
	}

	if (_right != 0.0f || _forward != 0.0f)
		_bodyInter->ActivateBody(_playerBodyID);

	_bodyInter->SetAngularVelocity(_playerBodyID, Vec3(0.0f, _right * 2.0f, 0.0f));

	Vec3 currentVel = _bodyInter->GetLinearVelocity(_playerBodyID);


	Vec3 horizontalVel(currentVel.GetX(), 0.0f, currentVel.GetZ());
	float speed = horizontalVel.Length();

	const float maxSpeed = 80.0f;          
	const float maxReverseSpeed = 40.0f;   
	const float accel = 100.0f * deltaTime; 

	Quat rotation = _bodyInter->GetRotation(_playerBodyID);
	Vec3 forwardDir = rotation * Vec3(0, 0, 1);
	forwardDir.SetY(0);
	forwardDir = forwardDir.Normalized();

	float forwardSpeed = horizontalVel.Dot(forwardDir);

	bool canAccelerateForward = (_forward > 0.0f) && (forwardSpeed < maxSpeed);
	bool canAccelerateBackward = (_forward < 0.0f) && (forwardSpeed > -maxReverseSpeed);

	if (canAccelerateForward || canAccelerateBackward)
	{
		Vec3 addVel = forwardDir * (_forward * accel);
		_bodyInter->AddLinearVelocity(_playerBodyID, addVel);
	}

}
