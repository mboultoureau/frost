#include "Cargo.h"

#include "../Game.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include "Frost.h"
#include "Frost/Scene/Components/RigidBody.h"

#include "../Physics/PhysicsLayer.h"

Cargo::Cargo()
{
	using namespace Frost;
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_cargo = scene.CreateGameObject("Cargo");
	scene.AddComponent<Transform>(
		_cargo,
		Transform::Vector3{ 0.0f, 2.0f, 20.0f },
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, 
		Transform::Vector3{ 5.0f, 5.0f, 5.0f }
	);
	scene.AddComponent<WorldTransform>(_cargo);
	scene.AddComponent<ModelRenderer>(_cargo, "./resources/meshes/cube.fbx");

	Vec3 position(0.0f, 10.0f, 20.0f);
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(5.0f, 5.0f, 5.0f)).Create().Get();
	BodyCreationSettings cargoBodySettings(boxShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::CARGO);
	scene.AddComponent<RigidBody>(_cargo, cargoBodySettings, _cargo, EActivation::Activate);
}
