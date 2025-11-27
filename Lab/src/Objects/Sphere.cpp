#include "Sphere.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Sphere::Sphere()
{
    Scene& _scene = Game::GetScene();

    _sphere = _scene.CreateGameObject("Sphere");
    _scene.AddComponent<Transform>(_sphere, Vector3{ 0.0f, 0.0f, 0.0f });

    _SetupPhysics();
}

void
Sphere::_SetupPhysics()
{
    using namespace JPH;

    Scene& _scene = Game::GetScene();

    ShapeRefC sphereShape = SphereShapeSettings(1.0f).Create().Get();
    BodyCreationSettings bodySettings(
        sphereShape, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::BULLET);

    _bodyId = Physics::Get().CreateBody(bodySettings);
    Physics::AddBody(_bodyId->GetID(), EActivation::Activate);
}
