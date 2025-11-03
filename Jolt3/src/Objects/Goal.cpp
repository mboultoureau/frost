#include "Goal.h"

#include "../Game.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include "Frost.h"
#include "Frost/Scene/Components/RigidBody2.h"

#include "../Physics/PhysicsLayer.h"

#include <windows.h>

class GoalScript : public Frost::Script
{
public:
	void OnCollisionEnter(Frost::GameObject::Id otherObject) override
	{
		Scene& scene = Game::GetScene();

		auto otherRigidbody = scene.GetComponent<Frost::RigidBody2>(otherObject);
		if (!otherRigidbody) return;

		auto id = GetGameObject();

		auto rigidBody = scene.GetComponent<Frost::RigidBody2>(GetGameObject());
		if (!rigidBody) return;

		if (otherRigidbody->body->GetObjectLayer() == ObjectLayers::CARGO)
		{
			_score += 1;

			FT_INFO("Goal reached. Score: {}", _score);
			
			// Teleport randomly
			std::uniform_int_distribution<int> uniform{ -50, 50 };
			int x = uniform(Frost::Random::PRNG());
			int z = uniform(Frost::Random::PRNG());


			if (_score == MAX_SCORE)
			{
				FT_INFO("YOU WON!!!");
				Beep(523, 500);

				scene.DestroyGameObject(GetGameObject());
				return;
			}

			Physics::Get().body_interface->SetPosition(
				rigidBody->body->GetID(),
				JPH::RVec3(static_cast<float>(x), 20.0f, static_cast<float>(z)),
				JPH::EActivation::Activate
			);
		}
	}

private:
	unsigned int _score = 0;
	static inline unsigned int MAX_SCORE = 3;
};

Goal::Goal()
{
	using namespace Frost;
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_goal = scene.CreateGameObject("Goal");
	scene.AddComponent<Transform>(
		_goal,
		Transform::Vector3{ 0.0f, 0.2f, 50.0f },
		Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Transform::Vector3{ 5.0f, 5.0f, 5.0f }
	);
	scene.AddComponent<WorldTransform>(_goal);
	scene.AddComponent<ModelRenderer>(_goal, "./resources/meshes/pill.fbx");
	scene.AddScript<GoalScript>(_goal);

	Vec3 position(0.0f, 20.0f, 50.0f);
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(10.0f, 2.0f, 10.0f)).Create().Get();
	// Rotate the box to 90 degrees on the X axis
	Quat rotation = Quat::sRotation(Vec3::sAxisX(), JPH::DegreesToRadians(90.0f));

	BodyCreationSettings goalBodySettings(boxShape, position, rotation, EMotionType::Static, ObjectLayers::GOAL);
	goalBodySettings.mUserData = _goal;
	goalBodySettings.mIsSensor = true;
	_body = Physics::CreateBody(goalBodySettings);
	Physics::AddBody(_body->GetID(), EActivation::Activate);
	scene.AddComponent<RigidBody2>(_goal, _body);
}
