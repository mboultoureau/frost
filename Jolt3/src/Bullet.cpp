#include "Bullet.h"

Frost::GameObject::Id BulletFactory::operator()(Frost::Scene& scene, Vector3& dir, float speed) {
	auto sphere = scene.CreateGameObject("Sphere");
	scene.AddComponent<Frost::Transform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<Frost::WorldTransform>(sphere, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<Frost::MeshRenderer>(sphere, "./resources/meshes/Sphere.fbx");
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
}