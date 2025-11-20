#include "Player.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Scriptable.h"


#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <memory>
#include <DirectXMath.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class PlayerScript : public Frost::Script {
	float _forward = 0.0f;
	float _previousForward = 1.0f;
	float _right = 0.0f;
	float _up = 0.0f;
	JPH::BodyID _playerBodyID;
	JPH::BodyInterface* _bodyInter;

public:
	void OnInitialize() override
	{
		_playerBodyID = Game::GetScene().GetComponent<RigidBody>(GetGameObject())->physicBody->bodyId;
		_bodyInter = Physics::Get().body_interface;
	}

	void OnFixedUpdate(float deltaTime) override
	{
		ProcessInput(deltaTime);
		UpdatePhysics(deltaTime);
	}
private:
	void ProcessInput(float deltaTime)
	{
		auto& scene = Game::GetScene();

		_forward = 0.0f;
		_right = 0.0f;
		_up = 0.0f;
		if (Input::GetKeyboard().IsKeyDown(K_UP) || Input::GetKeyboard().IsKeyDown(K_W))
		{
			_forward = 1.0f;
		}
		else if (Input::GetKeyboard().IsKeyDown(K_DOWN) || Input::GetKeyboard().IsKeyDown(K_S))
		{
			_forward = -1.0f;
		}

		// Upward/Downward
		if (Input::GetKeyboard().IsKeyDown(K_SPACE))
		{
			_up = 1.0f;
		}
		else if (Input::GetKeyboard().IsKeyDown(K_SHIFT))
		{
			_up = -1.0f;
		}

		// Steering
		if (Input::GetKeyboard().IsKeyDown(K_LEFT) || Input::GetKeyboard().IsKeyDown(K_A))
		{
			_right = -1.0f * deltaTime * 50.0f;
		}
		else if (Input::GetKeyboard().IsKeyDown(K_RIGHT) || Input::GetKeyboard().IsKeyDown(K_D))
		{
			_right = 1.0f * deltaTime * 50.0f;
		}
	}

	void UpdatePhysics(float deltaTime)
	{
		using namespace JPH;
		using namespace DirectX;

		if (_right != 0.0f || _forward != 0.0f)
		{
			Physics::ActivateBody(_playerBodyID);
		}

		_bodyInter->SetAngularVelocity(_playerBodyID, Vec3(0.0f, _right * 2.0f, 0.0f));
		_bodyInter->SetLinearVelocity(_playerBodyID,
			_bodyInter->GetRotation(_playerBodyID) * Vec3(0.0f, _up * 100.0f, _forward * 100.0f)
		);
	}
};

Player::Player()
{
	Scene& scene = Game::GetScene();

	_player = scene.CreateGameObject("Player");
	scene.AddComponent<Transform>(
		_player,
		Vector3{ 0.0f, 0.0f, -10.0f },
		Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		Vector3{ 1.0f, 1.0f, 1.0f }
	);
	scene.AddComponent<WorldTransform>(_player);


	// Camera
	_camera = scene.CreateGameObject("Camera", _player);
	scene.AddComponent<Transform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<WorldTransform>(_camera, Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<Camera>(_camera);

	auto cameraComponent = scene.GetComponent<Camera>(_camera);
	cameraComponent->backgroundColor.r = 47.0f / 255.0f;
	cameraComponent->backgroundColor.g = 116.0f / 255.0f;
	cameraComponent->backgroundColor.b = 228.0f / 255.0f;
	cameraComponent->backgroundColor.a = 1.0f;
	cameraComponent->postEffects.push_back(std::make_shared<Frost::ScreenShakeEffect>());
	cameraComponent->postEffects.push_back(std::make_shared<Frost::RadialBlurEffect>());

	InitializePhysics();
}

void Player::InitializePhysics()
{
	using namespace JPH;

	FT_ENGINE_ASSERT(_player != GameObject::InvalidId, "Player GameObject is invalid");
	Scene& scene = Game::GetScene();

	// Create vehicle body
	RVec3 position(0, 0.0f, -10);
	JPH::ShapeRefC sphereShape = JPH::SphereShapeSettings(1.0f).Create().Get();
	BodyCreationSettings player_body_settings(sphereShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
	player_body_settings.mGravityFactor = 0.0f;
	scene.AddComponent<RigidBody>(_player, player_body_settings, _player, EActivation::Activate);

	auto _playerBodyID = scene.GetComponent<RigidBody>(_player)->physicBody->bodyId;
	auto _bodyInter = Physics::Get().body_interface;

	scene.AddScript<PlayerScript>(_player);
}