#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Transform.h"

#include <Jolt/Jolt.h>

namespace Frost
{
    class PhysicSystem : public System
    {
    public:
        PhysicSystem();
        void FixedUpdate(Scene& scene, float deltaTime) override;
        void LateUpdate(Scene& scene, float deltaTime) override;

    private:
        void _UpdateAllJoltBodies(Scene& scene, float deltaTime);
        
        void _HandleAwakeVector(Scene& scene, float deltaTime);
        void _HandleSleepVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionEnterVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionStayVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionRemovedVector(Scene& scene, float deltaTime);

        template<typename Func>
        static void _ExecuteOnScripts(Scene& scene, entt::entity entity, Func func)
        {
            auto& registry = scene.GetRegistry();

            if (!registry.valid(entity)) return;

            auto* scriptable = registry.try_get<Component::Scriptable>(entity);
            if (scriptable)
            {
                for (auto& script : scriptable->_scripts)
                {
                    if (script) func(script.get());
                }
            }
        }
    };
}


