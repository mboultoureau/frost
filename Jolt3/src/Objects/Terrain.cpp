#include "Terrain.h"
#include "../Physics/PhysicsLayer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include "Frost.h"
#include "Frost/Scene/Components/RigidBody.h"

Terrain::Terrain(Scene& scene)
{
	using namespace JPH;

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
	BodyCreationSettings cargoBodySettings(boxShape, position, Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
	scene.AddComponent<RigidBody>(_terrain, cargoBodySettings, _terrain, EActivation::Activate);
}
