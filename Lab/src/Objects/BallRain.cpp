#include "BallRain.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Scriptable.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class DropletScript : public Frost::Script
{
public:
    void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override
    {
        auto& rb = GetGameObject().GetComponent<RigidBody>();

        Physics::Get().body_interface->SetGravityFactor(rb.physicBody->bodyId, 0.0f);
        Physics::Get().body_interface->DeactivateBody(rb.physicBody->bodyId);
    }
};

BallRain::BallRain()
{
    for (int x = -scale / 2; x < scale / 2; x += dist)
        for (int z = -scale / 2; z < scale / 2; z += dist)
            InstantiateDroplet(static_cast<float>(x), static_cast<float>(z));
}

void
BallRain::InstantiateDroplet(float x, float z)
{
    using namespace JPH;

    Scene& scene = Game::GetScene();

    auto _droplet = scene.CreateGameObject("Player");
    _droplet.AddComponent<Transform>(
        Vector3{ x, (x + z) / 2.0f + 2 * scale, z }, Vector4{ 0.0f, 0.0f, 1.0f, 1.0f }, Vector3{ 5.0f, 5.0f, 5.0f });
    _droplet.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/sphere.fbx" });

    FT_ENGINE_ASSERT(_droplet != GameObject::InvalidId, "Droplet GameObject is invalid");

    // Create vehicle body
    RVec3 position(x, (x + z) / 2.0f, z);
    JPH::ShapeRefC sphereShape = JPH::SphereShapeSettings(0.5f).Create().Get();
    BodyCreationSettings droplet_body_settings(
        sphereShape, position, Quat::sIdentity(), EMotionType::Dynamic, ObjectLayers::PLAYER);
    droplet_body_settings.mGravityFactor = 2.0f;
    scene.AddComponent<RigidBody>(_droplet, droplet_body_settings, _droplet, EActivation::Activate);

    scene.AddScript<DropletScript>(_droplet);
}