#include "Player.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include "Frost/Scene/Components/RigidBody2.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>

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

	// Camera Pivot
	_cameraPivot = scene.CreateGameObject("Camera Pivot", _player);
	scene.AddComponent<Transform>(_cameraPivot, Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<WorldTransform>(_cameraPivot);

	// Camera
	_camera = scene.CreateGameObject("Camera", _cameraPivot);
	scene.AddComponent<Transform>(_camera, Transform::Vector3{ 0.0f, 10.0f, -20.0f });
	scene.AddComponent<WorldTransform>(_camera, Transform::Vector3{ 0.0f, 10.0f, -20.0f });
	scene.AddComponent<Camera>(_camera);

	auto cameraComponent = scene.GetComponent<Camera>(_camera);
	cameraComponent->backgroundColor[0] = 47.0f / 255.0f;
	cameraComponent->backgroundColor[1] = 116.0f / 255.0f;
	cameraComponent->backgroundColor[2] = 228.0f / 255.0f;
	cameraComponent->backgroundColor[3] = 1.0f;

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
	// TODO: be sexy and load the shape from player and cargo
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(5.0f, 5.0f, 5.0f)).Create().Get();
	BodyCreationSettings motorcycle_body_settings(boxShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
	motorcycle_body_settings.mUserData = _player;
	_playerBody = Physics::CreateBody(motorcycle_body_settings);
	Physics::AddBody(_playerBody->GetID(), EActivation::Activate);

	scene.AddComponent<RigidBody2>(_player, _playerBody);
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

	// Clamp values
	const float MAX_PITCH = DirectX::XM_PIDIV2;
	const float MIN_PITCH = -DirectX::XM_PIDIV2;

	if (_cameraPivotRotationX > MAX_PITCH)
	{
		_cameraPivotRotationX = MAX_PITCH;
	}
	else if (_cameraPivotRotationX < MIN_PITCH)
	{
		_cameraPivotRotationX = MIN_PITCH;
	}

	const float TWO_PI = DirectX::XM_2PI;

	if (_cameraPivotRotationY > TWO_PI)
	{
		_cameraPivotRotationY -= TWO_PI;
	}
	else if (_cameraPivotRotationY < 0.0f)
	{
		_cameraPivotRotationY += TWO_PI;
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
		Physics::ActivateBody(_playerBody->GetID());
	}

	auto currentVelocity = _playerBody->GetLinearVelocity();

	_playerBody->SetAngularVelocity(Vec3(0.0f, _right * 2.0f, 0.0f));
	_playerBody->SetLinearVelocity(
		_playerBody->GetRotation() * Vec3(0.0f, currentVelocity.GetY(), _forward * 100.0f)
	);

	Scene& scene = Game::GetScene();
	auto transform = scene.GetComponent<Transform>(_cameraPivot);
	
	// Convert euler to quaternion
	XMVECTOR quaternion = DirectX::XMQuaternionRotationRollPitchYaw(
		_cameraPivotRotationX,
		_cameraPivotRotationY,
		0.0f
	);

	DirectX::XMStoreFloat4(&transform->rotation, quaternion);
}

void Player::CleanupBullets()
{
	std::erase_if(_bullets, [](const std::unique_ptr<Bullet>& bullet) {
		return bullet->IsExpired();
	});
}
