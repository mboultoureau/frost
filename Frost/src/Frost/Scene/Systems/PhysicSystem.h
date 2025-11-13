#pragma once

#include "Frost/Scene/ECS/System.h"

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
		void LateUpdate(ECS& ecs, float deltaTime) override;

        void UpdateAllJoltBodies(ECS& ecs, float deltaTime);

    private:
        //void InitRigidBody(ECS& ecs, RigidBody* rb, GameObject::Id id);
        //void UpdateColliderRigidbodyLinks(ECS& ecs);

        void _HandleAwakeVector(ECS& ecs, float deltaTime);
        void _HandleSleepVector(ECS& ecs, float deltaTime);
        void _HandleOnCollisionEnterVector(ECS& ecs, float deltaTime);
        void _HandleOnCollisionStayVector(ECS& ecs, float deltaTime);
        void _HandleOnCollisionRemovedVector(ECS& ecs, float deltaTime);
    };


}


