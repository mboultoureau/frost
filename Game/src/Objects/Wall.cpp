#include "Wall.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "Water.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Wall::Wall()
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    _wall = scene.CreateGameObject("Plane");
    _wall.AddComponent<Transform>(
        Vector3{ -365, 100.0f, -130 }, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, Vector3{ 50.0f, 50.0f, 1.0f });
    _wall.AddComponent<WorldTransform>();
    _wall.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/cube.fbx" });

    _SetupPhysics();
}

void
Wall::_SetupPhysics()
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    ShapeRefC boxShape = BoxShapeSettings(Vec3(50.0f, 50.f, 1.0f)).Create().Get();
    BodyCreationSettings planeBodySettings(
        boxShape, RVec3(-365, 100.0f, -130), Quat::sIdentity(), EMotionType::Static, ObjectLayers::NON_MOVING);
    planeBodySettings.mUserData = static_cast<JPH::uint64>(reinterpret_cast<uintptr_t>(&_wall));
    _wall.AddComponent<RigidBody>(planeBodySettings, _wall, EActivation::DontActivate);
    auto bodyId = _wall.GetComponent<RigidBody>().physicBody->bodyId;
}