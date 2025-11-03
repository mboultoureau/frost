#pragma once

#include "Frost/Scene/ECS/System.h"
#include <Frost/Scene/Components/RigidBody.h>

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include "Frost/Physics/Physics.h"

namespace Frost
{
    class PhysicSystem : public System
    {
    public:
        PhysicSystem();
        void FixedUpdate(ECS& ecs, float deltaTime) override;
        void UpdateAllJoltBodies(ECS& ecs, float deltaTime);
        void UpdateAllJoltBodies2(ECS& ecs, float deltaTime);

    private:
        void InitRigidBody(ECS& ecs, RigidBody* rb, GameObject::Id id);
        //void UpdateColliderRigidbodyLinks(ECS& ecs);

        template<typename Func>
        void _HandleCollisionVector(Physics::PairIdCollisionVector& colVector, ECS& ecs, float deltaTime, Func&& callback);
    };


}


