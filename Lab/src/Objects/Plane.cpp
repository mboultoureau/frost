#include "Plane.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Frost;

Plane::Plane()
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_plane = scene.CreateGameObject("Plane");
	scene.AddComponent<Transform>(
		_plane,
		Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Transform::Vector3{ 500.0f, 1.0f, 500.0f }
	);
	scene.AddComponent<WorldTransform>(_plane);
	scene.AddComponent<ModelRenderer>(_plane, "./resources/meshes/cube.fbx");

	_SetupPhysics();

	Vec3 position(0.0f, 0.0f, 00.0f);

}

void Plane::_SetupPhysics()
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	ShapeRefC boxShape = BoxShapeSettings(Vec3(500.0f, 0.1f, 500.0f)).Create().Get();
	BodyCreationSettings planeBodySettings(boxShape, RVec3(0.0f, 0.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
	planeBodySettings.mUserData = _plane;
	_body = Physics::CreateBody(planeBodySettings);
	Physics::AddBody(_body->GetID(), EActivation::Activate);

	scene.AddComponent<RigidBody2>(_plane, _body);
}