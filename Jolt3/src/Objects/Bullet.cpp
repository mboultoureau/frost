#include "Bullet.h"
#include "../Game.h"

#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Math/Quat.h>

#include "Frost/Scene/Components/RigidBody2.h"
#include "../Physics/PhysicsLayer.h"


class BulletScript : public Frost::Script
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
			FT_INFO("Bullet hit cargo, applying velocity.");
			Physics::ActivateBody(otherRigidbody->body->GetID());
		}

		scene.DestroyGameObject(GetGameObject());
	}
};

Bullet::Bullet(Transform transform)
{
	using namespace Frost;
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_bullet = scene.CreateGameObject("Bullet");

	scene.AddComponent<Transform>(
		_bullet,
		transform.position,
		transform.rotation,
		Transform::Vector3{ 2.0f, 2.0f, 2.0f }
	);
	scene.AddComponent<WorldTransform>(_bullet);
	scene.AddComponent<ModelRenderer>(_bullet, "./resources/meshes/sphere.fbx");
	scene.AddScript<BulletScript>(_bullet);

	// Add physics
	RVec3 position(transform.position.x, transform.position.y, transform.position.z);
	ShapeRefC boxShape = SphereShapeSettings(2.0f).Create().Get();
	JPH::Quat rotation_quat(
		transform.rotation.x,
		transform.rotation.y,
		transform.rotation.z,
		transform.rotation.w
	);
	BodyCreationSettings bulletBodySettings(boxShape, position, rotation_quat, EMotionType::Dynamic, ObjectLayers::BULLET);
	bulletBodySettings.mUserData = _bullet;
	bulletBodySettings.mRestitution = 20.0f;
	_body = Physics::CreateBody(bulletBodySettings);
	Physics::AddBody(_body->GetID(), EActivation::Activate);

	Vec3 rotation(
		transform.rotation.x,
		transform.rotation.y,
		transform.rotation.z
	);

	_body->GetMotionProperties()->SetGravityFactor(0.0f);
	_body->SetLinearVelocity(rotation_quat * Vec3(0.0f, 0.0f, 20.0f));
	scene.AddComponent<RigidBody2>(_bullet, _body);

 	_lifetimeTimer.Start();
}

Bullet::~Bullet()
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	scene.RemoveComponent<RigidBody2>(_bullet);
	scene.RemoveComponent<ModelRenderer>(_bullet);
	scene.RemoveComponent<WorldTransform>(_bullet);
	scene.RemoveComponent<Transform>(_bullet);

	scene.DestroyGameObject(_bullet);

	Physics::RemoveBody(_body->GetID());
}

bool Bullet::IsExpired() const
{
	using namespace std::chrono_literals;
	Timer::Duration cDuration = _lifetimeTimer.GetDuration();
	Timer::Duration fiveSeconds = Timer::Duration{ 5s };
	
	return cDuration >= fiveSeconds;
}























/*
Frost::GameObject::Id BulletFactory::operator()(Frost::Scene& scene, Vector3& dir, float speed) {
	auto sphere = scene.CreateGameObject("Sphere");
	scene.AddComponent<Frost::Transform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<Frost::WorldTransform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<Frost::ModelRenderer>(sphere, "./resources/meshes/Sphere.fbx");
	//TODO : scene.AddComponent<Frost::Body>(sphere);
	
	//scene.AddScript<BulletScript>(sphere, 5.f, 10.f, Vector3{ 0,0,1 });
	return sphere;
}

Frost::GameObject::Id BulletFactory::operator()(Frost::Scene& scene, Vector3& dir, float speed, Frost::GameObject::Id& parentId) {
	auto sphere = (*this)(scene, dir, speed);

	auto sphereInfo = scene.GetComponent<Frost::GameObjectInfo>(sphere);
	sphereInfo->parentId = parentId;

	return sphere;
}

Frost::GameObject::Id BulletFactory::operator()(Frost::Scene& scene, Vector3& dir, float speed, Frost::GameObjectInfo& parent) {
	auto sphere = (*this)(scene, dir, speed);

	auto sphereInfo = scene.GetComponent<Frost::GameObjectInfo>(sphere);
	sphereInfo->parentId = parent.id;

	return sphere;
}



void BulletScript::OnAwake() {
	//_goInfo = GetECS()->GetComponent<Frost::GameObjectInfo>(GetGameObject());
	//_body = GetECS()->GetComponent<Frost::Body>(_goInfo->id);
	//_body->speed = _dir * _speed;
}

void BulletScript::OnUpdate(float deltaTime) {
	_timer += deltaTime;
	if (_timer > _lifeDuration) {
		GetECS()->DestroyGameObject(_goInfo.id);
		return;
	}
}*/
