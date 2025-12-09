#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Relationship.h"
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
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Frost::Component;

namespace Frost
{
    static JPH::Ref<JPH::Shape> CreateJoltShape(const Component::CollisionShapeConfig& config,
                                                const Math::Vector3& scale)
    {
        JPH::ShapeSettings::ShapeResult result;

        JPH::Vec3 s = { abs(scale.x), abs(scale.y), abs(scale.z) };

        float minScale = 0.001f;
        if (s.GetX() < minScale)
            s.SetX(minScale);
        if (s.GetY() < minScale)
            s.SetY(minScale);
        if (s.GetZ() < minScale)
            s.SetZ(minScale);

        std::visit(
            [&](auto&& arg)
            {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, Component::ShapeBox>)
                {
                    JPH::Vec3 newHalfExtent = Math::vector_cast<JPH::Vec3>(arg.halfExtent) * s;

                    float minScaleFactor = std::min({ s.GetX(), s.GetY(), s.GetZ() });
                    float scaledConvexRadius = arg.convexRadius * minScaleFactor;

                    float minDimension = std::min({ newHalfExtent.GetX(), newHalfExtent.GetY(), newHalfExtent.GetZ() });
                    if (scaledConvexRadius >= minDimension)
                    {
                        scaledConvexRadius = minDimension * 0.9f;
                    }

                    JPH::BoxShapeSettings settings(newHalfExtent, scaledConvexRadius);
                    result = settings.Create();
                }
                else if constexpr (std::is_same_v<T, Component::ShapeSphere>)
                {
                    float maxScale = std::max({ s.GetX(), s.GetY(), s.GetZ() });
                    JPH::SphereShapeSettings settings(arg.radius * maxScale);
                    result = settings.Create();
                }
                else if constexpr (std::is_same_v<T, Component::ShapeCapsule>)
                {
                    float radiusScale = std::max(s.GetX(), s.GetZ());
                    float heightScale = s.GetY();

                    JPH::CapsuleShapeSettings settings(arg.halfHeight * heightScale, arg.radius * radiusScale);
                    result = settings.Create();
                }
                else if constexpr (std::is_same_v<T, Component::ShapeCylinder>)
                {
                    float radiusScale = std::max(s.GetX(), s.GetZ());
                    float heightScale = s.GetY();
                    float newRadius = arg.radius * radiusScale;

                    float scaledConvexRadius = arg.convexRadius * std::min(radiusScale, heightScale);

                    if (scaledConvexRadius >= newRadius)
                    {
                        scaledConvexRadius = newRadius * 0.9f;
                    }

                    JPH::CylinderShapeSettings settings(arg.halfHeight * heightScale, newRadius, scaledConvexRadius);
                    result = settings.Create();
                }
            },
            config);

        if (result.IsValid())
            return result.Get();

        FT_ENGINE_ERROR("PhysicSystem: Failed to create shape (Scale: {0}, {1}, {2})", scale.x, scale.y, scale.z);
        return nullptr;
    }

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

        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody, Component::Transform>(entity.GetHandle()))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity.GetHandle());
        auto& transform = registry.get<Component::Transform>(entity.GetHandle());

        if (rb.runtimeBodyID.IsInvalid())
        {
            _CreateBodyForEntity(scene, entity.GetHandle());
            return;
        }

        JPH::BodyInterface& bodyInterface = Physics::GetBodyInterface();

        bodyInterface.SetPositionAndRotation(rb.runtimeBodyID,
                                             Math::vector_cast<JPH::RVec3>(transform.position),
                                             Math::vector_cast<JPH::Quat>(transform.rotation),
                                             JPH::EActivation::DontActivate);

        JPH::Ref<JPH::Shape> newShape = CreateJoltShape(rb.shape, transform.scale);

        if (newShape)
        {
            bodyInterface.SetShape(rb.runtimeBodyID, newShape, true, JPH::EActivation::DontActivate);
        }

        bodyInterface.SetMotionType(
            rb.runtimeBodyID, static_cast<JPH::EMotionType>(rb.motionType), JPH::EActivation::DontActivate);
        bodyInterface.SetFriction(rb.runtimeBodyID, rb.friction);
        bodyInterface.SetRestitution(rb.runtimeBodyID, rb.restitution);
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
            return;

        JPH::Ref<JPH::Shape> finalShape = CreateJoltShape(rb.shape, transform.scale);

        if (!finalShape)
        {
            FT_ENGINE_ERROR("PhysicSystem: Failed to create shape for entity {0}", (uint32_t)entity);
            return;
        }

        JPH::BodyCreationSettings bodySettings(finalShape,
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

                if (rb.motionType == Component::RigidBody::MotionType::Static)
                    return;

                if (Physics::Get().body_interface->IsActive(rb.runtimeBodyID))
                {
                    auto jBodyPos = Physics::Get().body_interface->GetPosition(rb.runtimeBodyID);
                    auto jBodyRot = Physics::Get().body_interface->GetRotation(rb.runtimeBodyID);

                    transform.position = Math::vector_cast<Math::Vector3>(jBodyPos);
                    transform.rotation.x = jBodyRot.GetX();
                    transform.rotation.y = jBodyRot.GetY();
                    transform.rotation.z = jBodyRot.GetZ();
                    transform.rotation.w = jBodyRot.GetW();
                }
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
