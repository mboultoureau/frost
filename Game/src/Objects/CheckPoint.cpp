#include "CheckPoint.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include "Frost/Scene/Components/Script.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include <DirectXMath.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <algorithm>
#include <cmath>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

const float CHECKPOINT_SIZE = 5.0f;

class CheckPointScript : public Script
{
private:
    CheckPoint* checkPoint;

public:
    CheckPointScript(CheckPoint* checkpoint) : checkPoint{ checkpoint } {}
    virtual void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override
    {
        checkPoint->ReinitializeChildrenPhysics();

        auto parents = checkPoint->GetParent();

        if (parents.empty())
        {
            checkPoint->DestroyGameObject();
        }
        else
        {
            for (auto parent : parents)
            {
                parent->DeleteChildrenPhysics();
            }
        }

        CheckPoint::lastCheckPoint = GetGameObject();
    }
};

CheckPoint::CheckPoint(Vector3 startpos)
{
    Scene& scene = Game::GetScene();

    _checkpoint = scene.CreateGameObject("CheckPoint");
    _checkpoint.AddComponent<Transform>(startpos, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, Vector3{ 5.0f, 5.0f, 5.0f });
    _checkpoint.AddComponent<WorldTransform>();
}

void
CheckPoint::FixedUpdate(float deltaTime)
{
}

void
CheckPoint::AddChild(std::shared_ptr<CheckPoint> child)
{
    _nextCheckPoints.push_back(child);
}

void
CheckPoint::AddParent(std::shared_ptr<CheckPoint> parent)
{
    _prevCheckPoints.push_back(parent);
}

void
CheckPoint::InitializePhysics()
{
    using namespace JPH;

    FT_ENGINE_ASSERT(_checkpoint != GameObject::InvalidId, "CheckPoint GameObject is invalid");
    Scene& scene = Game::GetScene();

    auto& transform = _checkpoint.GetComponent<Transform>();
    RVec3 position = Math::vector_cast<Vec3>(transform.position);
    JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(5.0f, 5.0f, 5.0f)).Create().Get();
    BodyCreationSettings checkpoint_body_settings(
        boxShape, position, Quat::sIdentity(), EMotionType::Static, ObjectLayers::CHECKPOINT);

    checkpoint_body_settings.mGravityFactor = 0.0f;
    checkpoint_body_settings.mIsSensor = true;

    _checkpoint.AddComponent<RigidBody>(checkpoint_body_settings, _checkpoint, EActivation::Activate);
}

void
CheckPoint::ProcessInput(float deltaTime)
{
}

void
CheckPoint::UpdatePhysics(float deltaTime)
{
}

void
CheckPoint::ReinitializeChildrenPhysics()
{
    Scene& scene = Game::GetScene();

    for (std::shared_ptr<CheckPoint> child : _nextCheckPoints)
    {
        child->ActivatePhysics();
    }
}

void
CheckPoint::DeleteChildrenPhysics()
{
    using namespace JPH;
    Scene& scene = Game::GetScene();

    // Recuperation de l'interface Jolt
    JPH::BodyInterface* bodyInter = Physics::Get().body_interface;

    for (std::shared_ptr<CheckPoint> child : _nextCheckPoints)
    {
        GameObject childId = child->_checkpoint;

        if (!childId.HasComponent<Transform>())
        {
            continue;
        }

        if (childId.HasComponent<RigidBody>())
        {
            RigidBody& bodyComponent = childId.GetComponent<RigidBody>();
            JPH::BodyID bodyId = bodyComponent.physicBody->bodyId;

            childId.RemoveComponent<RigidBody>();

            childId.RemoveComponent<StaticMesh>();

            childId.RemoveComponent<CheckPointScript>();
        }
    }
}

void
CheckPoint::DestroyGameObject()
{
    using namespace JPH;
    Scene& scene = Game::GetScene();
    GameObject id = _checkpoint;

    if (id.HasComponent<RigidBody>())
    {
        RigidBody& bodyComponent = id.GetComponent<RigidBody>();
        JPH::BodyInterface* bodyInter = Physics::Get().body_interface;
        JPH::BodyID bodyId = bodyComponent.physicBody->bodyId;

        if (bodyId.IsInvalid() == false)
        {
            if (bodyInter->IsAdded(bodyId))
            {
                bodyInter->RemoveBody(bodyId);
            }
            bodyInter->DestroyBody(bodyId);
        }
    }

    scene.DestroyGameObject(id);
}
void
CheckPoint::ActivatePhysics()
{
    Scene& scene = Game::GetScene();
    if (!_checkpoint.HasComponent<StaticMesh>())
    {
        _checkpoint.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/sphere.fbx" });
    }
    if (_checkpoint.HasComponent<RigidBody>())
    {
        return;
    }
    InitializePhysics();
    _checkpoint.AddScript<CheckPointScript>(this);
}

GameObject CheckPoint::lastCheckPoint{ GameObject::InvalidId };