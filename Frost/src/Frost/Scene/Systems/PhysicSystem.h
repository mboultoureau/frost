#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Scene.h"

#include <entt/entt.hpp>

namespace Frost
{
    class PhysicSystem : public System
    {
    public:
        PhysicSystem() = default;
        virtual ~PhysicSystem() = default;

        virtual void OnAttach(Scene& scene) override;
        virtual void OnDetach(Scene& scene) override;
        virtual void FixedUpdate(Scene& scene, float fixedDeltaTime) override;
        virtual void LateUpdate(Scene& scene, float deltaTime) override;

        void NotifyRigidBodyUpdate(Scene& scene, GameObject entity);

    private:
        void _CreateBodyForEntity(Scene& scene, entt::entity entity);
        void _DestroyBodyForEntity(Scene& scene, entt::entity entity);
        void _OnCreateBody(entt::registry& registry, entt::entity entity);
        void _OnDestroyBody(entt::registry& registry, entt::entity entity);

        void _SynchronizeTransforms(Scene& scene);

        void _HandleAwakeVector(Scene& scene, float deltaTime);
        void _HandleSleepVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionEnterVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionStayVector(Scene& scene, float deltaTime);
        void _HandleOnCollisionRemovedVector(Scene& scene, float deltaTime);

        template<typename Func>
        void _ExecuteOnScripts(Scene& scene, entt::entity entity, Func func);

    private:
        Scene* _scene = nullptr;
    };

    template<typename Func>
    void PhysicSystem::_ExecuteOnScripts(Scene& scene, entt::entity entity, Func func)
    {
        if (scene.GetRegistry().valid(entity) && scene.GetRegistry().all_of<Component::Scriptable>(entity))
        {
            auto& scriptableComponent = scene.GetRegistry().get<Component::Scriptable>(entity);

            for (const auto& scriptInstance : scriptableComponent._scripts)
            {
                if (scriptInstance)
                {
                    func(scriptInstance.get());
                }
            }
        }
    }
} // namespace Frost