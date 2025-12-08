#include "Boost.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void
BoostScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
    auto id1 = params.inBody1.GetID();
    auto id2 = params.inBody2.GetID();

    if (id1 == _bodyId)
    {
        Physics::GetBodyInterface().AddForce(id2, _dir);
        return;
    }
    Physics::GetBodyInterface().AddForce(id1, _dir);
}

Boost::Boost(Vector3 pos, EulerAngles rot, Vector3 scale, Frost::Math::Vector3 dir, float power)
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    boost = scene.CreateGameObject("Boost");
    boost.AddComponent<Transform>(pos, rot, scale);
    boost.AddComponent<StaticMesh>(MeshSourceCube{ 1.0f });

    // Create water sensor. We use this to detect which bodies entered the water
    // (in this sample we could have assumed everything is in the water)
    RigidBody rbComp{ ShapeBox{ Vector3{ scale * 0.5f } }, ObjectLayers::WATER, RigidBody::MotionType::Static };
    rbComp.isSensor = true;
    auto& rb = boost.AddComponent<RigidBody>(rbComp);
    boost.AddScript<BoostScript>(dir, 15000.0f * power);
}
