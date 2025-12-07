#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/RelationShip.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Scripting/Script.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Utils/Math/Matrix.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Frost::Component;

namespace Frost
{
    void PhysicSystem::OnAttach(Scene& scene)
    {
        _scene = &scene;

        // Subscribe to changes
        auto& registry = scene.GetRegistry();
        registry.on_construct<Component::RigidBody>().connect<&PhysicSystem::_OnCreateBody>(*this);
        registry.on_destroy<Component::RigidBody>().connect<&PhysicSystem::_OnDestroyBody>(*this);

        auto view = scene.GetRegistry().view<RigidBody>();
        for (auto entity : view)
        {
            _CreateBodyForEntity(scene, entity);
        }
    }

    void PhysicSystem::OnDetach(Scene& scene)
    {
        // Unsubscribe to changes
        auto& registry = scene.GetRegistry();
        registry.on_construct<Component::RigidBody>().disconnect<&PhysicSystem::_OnCreateBody>(*this);
        registry.on_destroy<Component::RigidBody>().disconnect<&PhysicSystem::_OnDestroyBody>(*this);

        auto view = scene.GetRegistry().view<RigidBody>();
        for (auto entity : view)
        {
            _DestroyBodyForEntity(scene, entity);
        }
    }

    void PhysicSystem::FixedUpdate(Scene& scene, float fixedDeltaTime)
    {
        Physics::Get().UpdatePhysics(fixedDeltaTime);

        _SynchronizeTransforms(scene);

        _HandleAwakeVector(scene, fixedDeltaTime);
        _HandleSleepVector(scene, fixedDeltaTime);
        _HandleOnCollisionEnterVector(scene, fixedDeltaTime);
        _HandleOnCollisionStayVector(scene, fixedDeltaTime);
        _HandleOnCollisionRemovedVector(scene, fixedDeltaTime);
    }

    void PhysicSystem::LateUpdate(Scene& scene, float deltaTime)
    {
#ifdef FT_DEBUG
        Physics::Get().DrawDebug();
#endif
    }

    void PhysicSystem::NotifyRigidBodyUpdate(Scene& scene, GameObject entity)
    {
        if (!scene.GetRegistry().valid(entity.GetHandle()))
            return;

        _DestroyBodyForEntity(scene, entity.GetHandle());
        _CreateBodyForEntity(scene, entity.GetHandle());
    }

    void PhysicSystem::_CreateBodyForEntity(Scene& scene, entt::entity entity)
    {
        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody, Component::Transform>(entity))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity);
        auto& transform = registry.get<Component::Transform>(entity);
        auto& body_interface = Physics::GetBodyInterface();

        if (!rb.runtimeBodyID.IsInvalid())
        {
            return;
        }

        JPH::Ref<JPH::ShapeSettings> shapeSettings;
        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, Component::ShapeBox>)
                    shapeSettings =
                        new JPH::BoxShapeSettings(Math::vector_cast<JPH::Vec3>(arg.halfExtent), arg.convexRadius);
                else if constexpr (std::is_same_v<T, Component::ShapeSphere>)
                    shapeSettings = new JPH::SphereShapeSettings(arg.radius);
                else if constexpr (std::is_same_v<T, Component::ShapeCapsule>)
                    shapeSettings = new JPH::CapsuleShapeSettings(arg.halfHeight, arg.radius);
                else if constexpr (std::is_same_v<T, Component::ShapeCylinder>)
                    shapeSettings = new JPH::CylinderShapeSettings(arg.halfHeight, arg.radius, arg.convexRadius);
            },
            rb.shape);

        JPH::Shape::ShapeResult shapeResult = shapeSettings->Create();
        if (shapeResult.HasError())
        {
            FT_ENGINE_ERROR("PhysicSystem: Failed to create shape for entity {0}: {1}",
                            (uint32_t)entity,
                            shapeResult.GetError().c_str());
            return;
        }

        JPH::BodyCreationSettings bodySettings(shapeResult.Get(),
                                               Math::vector_cast<JPH::RVec3>(transform.position),
                                               Math::vector_cast<JPH::Quat>(transform.rotation),
                                               static_cast<JPH::EMotionType>(rb.motionType),
                                               rb.objectLayer);

        bodySettings.mIsSensor = rb.isSensor;
        bodySettings.mAllowSleeping = rb.allowSleeping;
        bodySettings.mFriction = rb.friction;
        bodySettings.mRestitution = rb.restitution;
        bodySettings.mLinearDamping = rb.linearDamping;
        bodySettings.mAngularDamping = rb.angularDamping;
        bodySettings.mGravityFactor = rb.gravityFactor;

        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs::All;
        if (rb.lockPositionX)
            dofs &= ~JPH::EAllowedDOFs::TranslationX;
        if (rb.lockPositionY)
            dofs &= ~JPH::EAllowedDOFs::TranslationY;
        if (rb.lockPositionZ)
            dofs &= ~JPH::EAllowedDOFs::TranslationZ;
        if (rb.lockRotationX)
            dofs &= ~JPH::EAllowedDOFs::RotationX;
        if (rb.lockRotationY)
            dofs &= ~JPH::EAllowedDOFs::RotationY;
        if (rb.lockRotationZ)
            dofs &= ~JPH::EAllowedDOFs::RotationZ;
        bodySettings.mAllowedDOFs = dofs;

        if (rb.motionType == Component::RigidBody::MotionType::Dynamic)
        {
            bodySettings.mOverrideMassProperties = static_cast<JPH::EOverrideMassProperties>(rb.overrideMassProperties);
            if (rb.overrideMassProperties != Component::RigidBody::OverrideMassProperties::CalculateMassAndInertia)
                bodySettings.mMassPropertiesOverride.mMass = rb.mass;
        }

        bodySettings.mUserData = static_cast<uint64_t>(entity);

        JPH::Body* body = body_interface.CreateBody(bodySettings);
        if (body)
        {
            rb.runtimeBodyID = body->GetID();
            body_interface.AddBody(rb.runtimeBodyID, JPH::EActivation::Activate);
        }
        else
        {
            FT_ENGINE_ERROR("PhysicSystem: Failed to create body for entity {0}", (uint32_t)entity);
        }
    }

    void PhysicSystem::_DestroyBodyForEntity(Scene& scene, entt::entity entity)
    {
        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody>(entity))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity);
        if (rb.runtimeBodyID.IsInvalid())
            return;

        auto& body_interface = Physics::GetBodyInterface();
        body_interface.RemoveBody(rb.runtimeBodyID);
        body_interface.DestroyBody(rb.runtimeBodyID);
        rb.runtimeBodyID = JPH::BodyID();
    }

    void PhysicSystem::_OnCreateBody(entt::registry& registry, entt::entity entity)
    {
        _CreateBodyForEntity(*_scene, entity);
    }

    void PhysicSystem::_OnDestroyBody(entt::registry& registry, entt::entity entity)
    {
        _DestroyBodyForEntity(*_scene, entity);
    }

    void PhysicSystem::_SynchronizeTransforms(Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        auto& body_interface = Physics::GetBodyInterface();

        auto view = registry.view<Component::RigidBody, Component::Transform>();

        view.each(
            [&](auto entity, Component::RigidBody& rb, Component::Transform& transform)
            {
                if (rb.runtimeBodyID.IsInvalid())
                    return;

                auto jBodyPos = Physics::Get().body_interface->GetPosition(rb.runtimeBodyID);
                auto jBodyRot = Physics::Get().body_interface->GetRotation(rb.runtimeBodyID);

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
            _ExecuteOnScripts(scene, entity, [&](Scripting::Script* script) { script->OnAwake(deltaTime); });
        }
        physics.bodiesOnAwake.clear();
    }

    void Frost::PhysicSystem::_HandleSleepVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnSleep)
        {
            entt::entity entity = Physics::GetEntityID(params.inBodyID);
            _ExecuteOnScripts(scene, entity, [&](Scripting::Script* script) { script->OnSleep(deltaTime); });
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

            _ExecuteOnScripts(
                scene, entity1, [&](Scripting::Script* script) { script->OnCollisionEnter(params, deltaTime); });

            _ExecuteOnScripts(
                scene, entity2, [&](Scripting::Script* script) { script->OnCollisionEnter(params, deltaTime); });
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

            _ExecuteOnScripts(
                scene, entity1, [&](Scripting::Script* script) { script->OnCollisionStay(params, deltaTime); });

            _ExecuteOnScripts(
                scene, entity2, [&](Scripting::Script* script) { script->OnCollisionStay(params, deltaTime); });

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
                    scene, entity1, [&](Scripting::Script* script) { script->OnCollisionExit(exitParams, deltaTime); });

                _ExecuteOnScripts(
                    scene, entity2, [&](Scripting::Script* script) { script->OnCollisionExit(exitParams, deltaTime); });
            }
        }

        // Swap buffers for next frame
        physics.lastFrameBodyIDsOnCollisionStay = physics.currentFrameBodyIDsOnCollisionStay;
        physics.currentFrameBodyIDsOnCollisionStay.clear();
    }

} // namespace Frost
