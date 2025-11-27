#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Components/Script.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Frost::Component;

namespace Frost
{

    Frost::PhysicSystem::PhysicSystem() {}

    void Frost::PhysicSystem::FixedUpdate(Scene& scene, float deltaTime)
    {
        Physics& physic = Physics::Get();

        physic.UpdatePhysics();

        _UpdateAllJoltBodies(scene, deltaTime);

        // Awake and sleep
        _HandleAwakeVector(scene, deltaTime);
        _HandleSleepVector(scene, deltaTime);

        // Collisions
        _HandleOnCollisionEnterVector(scene, deltaTime);
        _HandleOnCollisionStayVector(scene, deltaTime);
        _HandleOnCollisionRemovedVector(scene, deltaTime);
    }

    void Frost::PhysicSystem::LateUpdate(Scene& scene, float deltaTime)
    {
#ifdef FT_DEBUG
        Physics::DrawDebug();
#endif
    }

    void Frost::PhysicSystem::_UpdateAllJoltBodies(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        auto rigidBodyView = scene.ViewActive<RigidBody, Transform>();

        rigidBodyView.each(
            [&](const RigidBody& rbody, Transform& transform)
            {
                auto jBodyPos = Physics::Get().body_interface->GetPosition(rbody.physicBody->bodyId);
                auto jBodyRot = Physics::Get().body_interface->GetRotation(rbody.physicBody->bodyId);

                transform.position = Math::vector_cast<Math::Vector3>(jBodyPos);
                transform.rotation.x = jBodyRot.GetX();
                transform.rotation.y = jBodyRot.GetY();
                transform.rotation.z = jBodyRot.GetZ();
                transform.rotation.w = jBodyRot.GetW();
            });
    }

    void Frost::PhysicSystem::_HandleAwakeVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnAwake)
        {
            entt::entity entity = Physics::GetEntityID(params.inBodyID);
            _ExecuteOnScripts(scene, entity, [&](Script* script) { script->OnAwake(deltaTime); });
        }
        physics.bodiesOnAwake.clear();
    }

    void Frost::PhysicSystem::_HandleSleepVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnSleep)
        {
            entt::entity entity = Physics::GetEntityID(params.inBodyID);
            _ExecuteOnScripts(scene, entity, [&](Script* script) { script->OnSleep(deltaTime); });
        }
        physics.bodiesOnSleep.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionEnterVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();

        for (const auto& params : physics.bodiesOnCollisionEnter)
        {
            entt::entity entity1 = Physics::GetEntityID(params.inBody1.GetID());
            entt::entity entity2 = Physics::GetEntityID(params.inBody2.GetID());

            _ExecuteOnScripts(scene, entity1, [&](Script* script) { script->OnCollisionEnter(params, deltaTime); });

            _ExecuteOnScripts(scene, entity2, [&](Script* script) { script->OnCollisionEnter(params, deltaTime); });
        }
        physics.bodiesOnCollisionEnter.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionStayVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnCollisionStay)
        {
            entt::entity entity1 = Physics::GetEntityID(params.inBody1.GetID());
            entt::entity entity2 = Physics::GetEntityID(params.inBody2.GetID());

            _ExecuteOnScripts(scene, entity1, [&](Script* script) { script->OnCollisionStay(params, deltaTime); });

            _ExecuteOnScripts(scene, entity2, [&](Script* script) { script->OnCollisionStay(params, deltaTime); });

            physics.currentFrameBodyIDsOnCollisionStay.emplace(entity1, entity2);
        }
        physics.bodiesOnCollisionStay.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionRemovedVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();

        for (const auto& pair : physics.lastFrameBodyIDsOnCollisionStay)
        {
            if (physics.currentFrameBodyIDsOnCollisionStay.find(pair) ==
                physics.currentFrameBodyIDsOnCollisionStay.end())
            {
                entt::entity entity1 = pair.first;
                entt::entity entity2 = pair.second;

                std::pair<entt::entity, entt::entity> exitParams = { entity1, entity2 };

                _ExecuteOnScripts(
                    scene, entity1, [&](Script* script) { script->OnCollisionExit(exitParams, deltaTime); });

                _ExecuteOnScripts(
                    scene, entity2, [&](Script* script) { script->OnCollisionExit(exitParams, deltaTime); });
            }
        }

        // Swap buffers for next frame
        physics.lastFrameBodyIDsOnCollisionStay = physics.currentFrameBodyIDsOnCollisionStay;
        physics.currentFrameBodyIDsOnCollisionStay.clear();
    }

} // namespace Frost
