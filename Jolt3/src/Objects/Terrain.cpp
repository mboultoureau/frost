#include "Terrain.h"
#include "../Physics/PhysicsLayer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include "Frost.h"
#include "Frost/Scene/Components/RigidBody.h"

Terrain::Terrain(Scene& scene)
{
	using namespace JPH;
	//Terrain
	_terrain = scene.CreateGameObject("Terrain");
	scene.AddComponent<Transform>(
		_terrain,
		Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Transform::Vector3{ 500.0f, 1.0f, 500.0f }
	);
	scene.AddComponent<WorldTransform>(_terrain, Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<ModelRenderer>(_terrain, "./resources/meshes/cube.fbx");

	Vec3 position(0.0f, 0.0f, 00.0f);
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(500.0f, 0.1f, 500.0f)).Create().Get();
	BodyCreationSettings terrainBodySettings(boxShape, position, Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
	scene.AddComponent<RigidBody>(_terrain, terrainBodySettings, _terrain, EActivation::Activate);

	//Wall
	
	_wall = scene.CreateGameObject("Wall");
	scene.AddComponent<Transform>(
		_wall,
		Transform::Vector3{ 200.0f, 25.0f, 0.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Transform::Vector3{ 10, 50.0f, 10 }
	);
	scene.AddComponent<WorldTransform>(_wall, Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	scene.AddComponent<ModelRenderer>(_wall, "./resources/meshes/cube.fbx");

	JPH::ShapeRefC wallShape = JPH::BoxShapeSettings(Vec3(10.0f, 50.0f, 10.0f)).Create().Get();
	BodyCreationSettings wallBodySettings(wallShape, position, Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
	scene.AddComponent<RigidBody>(_wall, wallBodySettings, _wall, EActivation::Activate);
	
}
