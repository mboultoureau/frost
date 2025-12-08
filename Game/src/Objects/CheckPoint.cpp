#include "CheckPoint.h"
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
#include "Frost/Scene/Components/Script.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "Billboard.h"
#include <DirectXMath.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <algorithm>
#include <cmath>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

// CONSTANTES MISES  JOUR
const float COLLISION_THICKNESS = 0.5f;
const float BILLBOARD_SIZE = 20.0f;
const float COLLISION_SCALE = 1.5f;
const std::string BILLBOARD_TEXTURE_PATH = "resources/textures/goal-flag.png";

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

GameObject CheckPoint::lastCheckPoint{ GameObject::InvalidId };

CheckPoint::CheckPoint(Vector3 startpos)
{
    Scene& scene = Game::GetScene();

    _checkpoint = scene.CreateGameObject("CheckPoint");

    _checkpoint.AddComponent<Transform>(startpos, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _checkpoint.AddComponent<WorldTransform>();
}

void
CheckPoint::FixedUpdate(float deltaTime)
{
    ProcessInput(deltaTime);
    UpdatePhysics(deltaTime);
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

    constexpr float CHECKPOINT_SIZE = 5.0f;

    auto& rb = _checkpoint.AddComponent<RigidBody>(ShapeSphere{ CHECKPOINT_SIZE }, ObjectLayers::CHECKPOINT);

    rb.gravityFactor = 0.0f;
    rb.isSensor = true;
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

    JPH::BodyInterface* bodyInter = Physics::Get().body_interface;

    for (std::shared_ptr<CheckPoint> child : _nextCheckPoints)
    {
        GameObject childId = child->_checkpoint;

        if (child->_flagBillboard)
        {
            child->_flagBillboard->DestroyObject();
            child->_flagBillboard.reset();
        }

        if (childId.HasComponent<RigidBody>())
        {
            RigidBody& bodyComponent = childId.GetComponent<RigidBody>();
            JPH::BodyID bodyId = bodyComponent.runtimeBodyID;

            if (bodyId.IsInvalid() == false && bodyInter->IsAdded(bodyId))
            {
                bodyInter->RemoveBody(bodyId);
            }

            childId.RemoveComponent<RigidBody>();
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

    if (_flagBillboard)
    {
        _flagBillboard->DestroyObject();
        _flagBillboard.reset();
    }

    if (id.HasComponent<RigidBody>())
    {
        RigidBody& bodyComponent = id.GetComponent<RigidBody>();
        JPH::BodyInterface* bodyInter = Physics::Get().body_interface;
        JPH::BodyID bodyId = bodyComponent.runtimeBodyID;

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

    if (!_flagBillboard && _checkpoint.HasComponent<Transform>())
    {
        auto& transform = _checkpoint.GetComponent<Transform>();
        _flagBillboard = std::make_unique<Billboard>(transform.position, BILLBOARD_SIZE, BILLBOARD_TEXTURE_PATH);
    }

    if (_checkpoint.HasComponent<RigidBody>())
    {
        return;
    }

    InitializePhysics();
    _checkpoint.AddScript<CheckPointScript>(this);
}
