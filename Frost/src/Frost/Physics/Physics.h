#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Utils/NoCopy.h"

#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include "Frost/Physics/Layers.h"
#include "Frost/Physics/PhysicListener.h"
#include "Frost/Physics/PhysicsConfig.h"
#include "Frost/Physics/ShapeRegistry.h"
#include "Frost/Renderer/Pipeline/JoltDebugRenderingPipeline.h"

#include "Frost/Utils/Math/Vector.h"

#include <map>
#include <mutex>
#include <set>
#include <vector>

#include <Frost/Renderer/DX11/TextureDX11.h>

namespace Frost
{
    class FROST_API Physics : NoCopy
    {
        friend class PhysicSystem;
        // ===== Constructors =================
    public:
        static Physics& Get();
        Physics();
        ~Physics();
        void UpdatePhysics(float fixedDeltaTime);
        static void InitPhysics(PhysicsConfig& config, bool useConfig);
        static void Shutdown();

        JPH::PhysicsSystem physics_system;
        JPH::BodyInterface* body_interface;

    private:
        JPH::TempAllocatorImpl temp_allocator;
        JPH::JobSystemThreadPool job_system;
        const int cCollisionSteps = 1;
        const JPH::uint cMaxBodies;
        const JPH::uint cNumBodyMutexes;
        const JPH::uint cMaxBodyPairs;
        const JPH::uint cMaxContactConstraints;
        inline static bool _physicsInitialized = false;
        inline static PhysicsConfig _physicsConfig;

        // ===== Debug Macros =================
    public:
        static void TraceImpl(const char* inFMT, ...);

#ifdef FT_DEBUG
    public:
        static void DrawDebug();
        // static Frost::DebugRendererPhysicsConfig& GetDebugRendererConfig();
        // static void SetDebugRendererConfig(Frost::DebugRendererPhysicsConfig
        // config);
#endif

#ifdef JPH_ENABLE_ASSERTS
    public:
        static bool AssertFailedImpl(const char* inExpression,
                                     const char* inMessage,
                                     const char* inFile,
                                     JPH::uint inLine);
#endif

        // ===== Body Interface =================
    public:
        static void AddConstraint(JPH::Constraint* inConstraint);
        static void AddConstraints(JPH::Constraint** inConstraints, int inNumber);
        static void AddStepListener(JPH::PhysicsStepListener* inListener);
        static JPH::Body* CreateBody(const JPH::BodyCreationSettings& inSettings);
        static void AddBody(const JPH::BodyID& inBodyID, JPH::EActivation inActivationMode);
        static JPH::BodyID CreateAndAddBody(JPH::BodyCreationSettings& inSettings,
                                            const entt::entity& rigidBodyId,
                                            const JPH::EActivation& inActivationMode);
        static JPH::Vec3 GetGravity();
        static void ActivateBody(const JPH::BodyID& inBodyID);
        static void RemoveAndDestroyBody(const JPH::BodyID& inBodyID);

        static JPH::BodyInterface& GetBodyInterface();
        static const JPH::BodyLockInterfaceLocking& GetBodyLockInterface();

        static entt::entity GetEntityID(const JPH::BodyID& inBodyID);

        void Clear() { Physics::Get().physics_system.~PhysicsSystem(); }
        static JPH::DebugRenderer* GetDebugRenderer();

        // ===== Layers Interface =================
        std::mutex mutexOnAwake;
        std::mutex mutexOnSleep;
        std::mutex mutexOnCollisionEnter;
        std::mutex mutexOnCollisionStay;

        std::vector<Frost::BodyActivationParameters> bodiesOnAwake = {};
        std::vector<Frost::BodyActivationParameters> bodiesOnSleep = {};
        std::vector<Frost::BodyOnContactParameters> bodiesOnCollisionEnter = {};
        std::vector<Frost::BodyOnContactParameters> bodiesOnCollisionStay = {};
        std::set<std::pair<entt::entity, entt::entity>> lastFrameBodyIDsOnCollisionStay = {};
        std::set<std::pair<entt::entity, entt::entity>> currentFrameBodyIDsOnCollisionStay = {};

        Frost::MyBodyActivationListener body_activation_listener;
        Frost::MyContactListener contact_listener;
        JPH::BroadPhaseLayerInterface* broad_phase_layer_interface;

    private:
        JPH::ObjectVsBroadPhaseLayerFilter* object_vs_broadphase_layer_filter;
        JPH::ObjectLayerPairFilter* object_vs_object_layer_filter;

#ifdef FT_DEBUG
        JPH::DebugRenderer* _debugRenderer;
#endif
    };
} // namespace Frost
