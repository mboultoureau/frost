#include "Bike.h"
#include "../../../Physics/PhysicsLayer.h"
#include "../../../Game.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/WorldTransform.h"

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
using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;


Bike::Bike(Player* player, RendererParameters params) : Vehicle( player, params)
{
	// Create motorcycle controller
	// Set playerbody to made body
	RenderMesh(false);
}

JPH::BodyID Bike::Appear()
{
	using namespace JPH;
	RenderMesh(true);

	auto wTransform = _scene->GetComponent<Transform>(_player->GetPlayerID());
	auto joltPos = Math::vector_cast<Vec3>(wTransform->position);
	auto joltRot = Math::vector_cast<Quat>(wTransform->rotation);

	JPH::ShapeRefC shape = new JPH::SphereShape(5.0f);

	BodyCreationSettings bodySettings(shape, joltPos, joltRot, EMotionType::Dynamic, ObjectLayers::PLAYER);
	bodySettings.mRestitution = 0.0f;
	bodySettings.mAllowSleeping = false;
	bodySettings.mFriction = 100.f;
	bodySettings.mAllowedDOFs =
		EAllowedDOFs::RotationY | EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
	_bodyId = Physics::CreateAndAddBody(bodySettings, _player->GetPlayerID(), EActivation::Activate);
	return _bodyId;
}

void Bike::Disappear()
{
	using namespace JPH;
	RenderMesh(false);
	Physics::RemoveAndDestroyBody(_bodyId);
}


void Bike::OnFixedUpdate(float deltaTime)
{

}

void Bike::OnLateUpdate(float deltaTime)
{
}

void Bike::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
}

void Bike::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
}

void Bike::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
}


void Bike::OnLeftRightInput(float deltaTime, float leftRightInput)
{
}

void Bike::OnAccelerateInput(float deltaTime, float upDownInput)
{
}

void Bike::OnBrake(float deltaTime)
{
}


/*

void Bike::InitializePhysics()
{
	using namespace JPH;

	FT_ENGINE_ASSERT(_player != GameObject::InvalidId, "Player GameObject is invalid");
	Scene& scene = Game::GetScene();

	// Create vehicle body
	RVec3 position(-365, 100.0f, -100);

	JPH::ShapeRefC capsuleShape = new JPH::CapsuleShape(5.0f, 1.0f);
	Quat localRot = Quat::sRotation(Vec3::sAxisX(), JPH::DegreesToRadians(90.0f));
	JPH::ShapeRefC rotatedCapsule = new RotatedTranslatedShape(Vec3(0, 0, 0), localRot, capsuleShape);

	//JPH::ShapeRefC boxShape = JPH::SphereShapeSettings(5.0f).Create().Get();
	BodyCreationSettings motorcycle_body_settings(rotatedCapsule, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
	motorcycle_body_settings.mRestitution = 0.0f;
	motorcycle_body_settings.mAllowSleeping = false;
	motorcycle_body_settings.mFriction = 100.f;
	motorcycle_body_settings.mAllowedDOFs =
		EAllowedDOFs::RotationY | EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
	scene.AddComponent<RigidBody>(_player, motorcycle_body_settings, _player, EActivation::Activate);

	_playerBodyID = scene.GetComponent<RigidBody>(_player)->physicBody->bodyId;
	_cameraBodyID = scene.GetComponent<RigidBody>(_playerCamera->_camera)->physicBody->bodyId;
	_bodyInter = Physics::Get().body_interface;

	_bodyInter->SetMotionQuality(_playerBodyID, EMotionQuality::Discrete);
}

void Bike::ProcessInput(float deltaTime)
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



void Bike::UpdatePhysics(float deltaTime)
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

*/
