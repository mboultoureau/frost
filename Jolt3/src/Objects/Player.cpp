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


Player::Player()
{
	Scene& scene = Game::GetScene();

	_player = scene.CreateGameObject("Player");
	scene.AddComponent<Transform>(
		_player,
		Transform::Vector3{ 0.0f, 2.0f, 0.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		Transform::Vector3{ 5.0f, 5.0f, 5.0f }
	);
	scene.AddComponent<WorldTransform>(_player);

	// Vehicle Model
	_vehicle = scene.CreateGameObject("Vehicle", _player);
	scene.AddComponent<Transform>(_vehicle, Transform::Vector3{ 0.0f, -1.5f, 0.0f });
	scene.AddComponent<WorldTransform>(_vehicle);
	scene.AddComponent<ModelRenderer>(_vehicle, "./resources/meshes/cube.fbx");

	//Add spring camera
	auto wt = scene.GetComponent<WorldTransform>(_player);
	auto pCam = PlayerCamera(_player);
	_playerCamera = &pCam;
	
	InitializePhysics();
	
	_fireTimer.Start();
}

void Player::FixedUpdate(float deltaTime)
{
	CleanupBullets();
	ProcessInput(deltaTime);
	UpdatePhysics(deltaTime);
}

void Player::InitializePhysics()
{
	using namespace JPH;

	FT_ENGINE_ASSERT(_player != GameObject::InvalidId, "Player GameObject is invalid");
	Scene& scene = Game::GetScene();

	// Create vehicle body
	RVec3 position(0, 10.0f, 0);
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(5.0f, 5.0f, 5.0f)).Create().Get();
	BodyCreationSettings motorcycle_body_settings(boxShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
	scene.AddComponent<RigidBody>(_player, motorcycle_body_settings, _player, EActivation::Activate);

	_playerBodyID = scene.GetComponent<RigidBody>(_player)->bodyId;
	//_cameraBodyID = _playerBodyID = scene.GetComponent<RigidBody>(_playerCamera->_camera)->bodyId;
	_bodyInter = Physics::Get().body_interface;
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

	// Fire bullets
	if (_fireTimer.GetDurationAs<std::chrono::milliseconds>().count() < 500)
	{
		return;
	}
	
	if (Input::GetKeyboard().IsKeyDown(K_SPACE))
	{
 		auto playerTransform = scene.GetComponent<Transform>(_player);
		_bullets.push_back(std::make_unique<Bullet>(*playerTransform));

		_fireTimer.Start();
	}
}

void Player::UpdatePhysics(float deltaTime)
{
	using namespace JPH;
	using namespace DirectX;

	if (_right != 0.0f || _forward != 0.0f)
	{
		Physics::ActivateBody(_playerBodyID);
		_bodyInter->ActivateBody(_cameraBodyID);
	}

	auto currentVelocity = _bodyInter->GetLinearVelocity(_playerBodyID);

	_bodyInter->SetAngularVelocity(_playerBodyID, Vec3(0.0f, _right * 2.0f, 0.0f));
	_bodyInter->SetLinearVelocity(_playerBodyID,
		_bodyInter->GetRotation(_playerBodyID) * Vec3(0.0f, currentVelocity.GetY(), _forward * 100.0f)
	);
}

void Player::CleanupBullets()
{
	std::erase_if(_bullets, [](auto& bullet) {
		return bullet->IsExpired();
	});
}