#include "Wall.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Wall::Wall()
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_wall = scene.CreateGameObject("Plane");
	scene.AddComponent<Transform>(
		_wall,
		Vector3{ -365, 100.0f, -130 },
		Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Vector3{ 50.0f, 50.0f, 1.0f }
	);
	scene.AddComponent<WorldTransform>(_wall);
	scene.AddComponent<StaticMesh>(_wall, "./resources/meshes/cube.fbx");

	_SetupPhysics();
}

void Wall::_SetupPhysics()
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	ShapeRefC boxShape = BoxShapeSettings(Vec3(50.0f, 50.f, 1.0f)).Create().Get();
	BodyCreationSettings planeBodySettings(boxShape, RVec3(-365, 100.0f, -130), Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
	planeBodySettings.mUserData = _wall;
	scene.AddComponent<RigidBody>(_wall, planeBodySettings, _wall, EActivation::DontActivate);
	auto bodyId = scene.GetComponent<RigidBody>(_wall)->physicBody->bodyId;
}