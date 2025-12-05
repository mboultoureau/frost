#include "Plane.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Plane::Plane()
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    _plane = scene.CreateGameObject("Plane");
    _plane.AddComponent<Transform>(
        Vector3{ 0.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, Vector3{ 500.0f, 1.0f, 500.0f });
    _plane.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/cube.fbx" });

    _plane.AddComponent<RigidBody>(ShapeBox{ Vector3{ 500.0f, 0.1f, 500.0f } }, ObjectLayers::NON_MOVING);

    Vec3 position(0.0f, 0.0f, 00.0f);
}
