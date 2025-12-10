#include "Frost/Physics/Physics.h"

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <cassert>
#include <iostream>
#include <stdarg.h>

#include "Frost/Utils/Math/Vector.h"
#include <Frost/Debugging/Assert.h>
#include <Frost/Debugging/Logger.h>
#include <Frost/Scene/Components/Meta.h>
#include <Frost/Scene/Systems/PhysicSystem.h>

using namespace JPH;
using namespace Frost::Math;

namespace Frost
{
    void Physics::InitPhysics(PhysicsConfig& config, bool useConfig)
    {
        FT_ENGINE_ASSERT(!_physicsInitialized, "Physics has already been initialized!");
        FT_ENGINE_INFO("Initializing Physics...");

        if (useConfig)
        {
            FT_ENGINE_ASSERT(config.broadPhaseLayerInterface, "broadPhaseLayerInterface is null!");
            FT_ENGINE_ASSERT(config.objectLayerPairFilter, "objectLayerPairFilter is null!");
            FT_ENGINE_ASSERT(config.objectVsBroadPhaseLayerFilter, "objectVsBroadPhaseLayerFilter is null!");
            _physicsConfig = config;
            _ownsConfigPointers = false;
        }
        else
        {
            _physicsConfig = PhysicsConfig{ new BPLayerInterfaceImpl{},
                                            new ObjectLayerPairFilterImpl{},
                                            new ObjectVsBroadPhaseLayerFilterImpl{} };
            _ownsConfigPointers = true;
        }

        JPH::RegisterDefaultAllocator();
        JPH::Trace = Physics::TraceImpl;
        JPH_IF_ENABLE_ASSERTS(AssertFailed = Physics::AssertFailedImpl;)
        JPH::Factory::sInstance = new JPH::Factory();
        RegisterTypes();

        _singleton = new Physics();

        _physicsInitialized = true;
    }

    void Physics::Shutdown()
    {
        if (!_physicsInitialized)
            return;

        FT_ENGINE_INFO("Shutting down Physics...");

        delete _singleton;
        _singleton = nullptr;

        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        if (_ownsConfigPointers)
        {
            delete _physicsConfig.broadPhaseLayerInterface;
            delete _physicsConfig.objectLayerPairFilter;
            delete _physicsConfig.objectVsBroadPhaseLayerFilter;
        }

        _physicsInitialized = false;
    }

    Physics& Physics::Get()
    {
        FT_ENGINE_ASSERT(_singleton, "Physics not initialized or already shut down. Call InitPhysics first.");
        return *_singleton;
    }

    Physics::Physics() :
        cMaxBodies(1024),
        cMaxBodyPairs(1024),
        cNumBodyMutexes(0),
        cMaxContactConstraints(1024),
        body_activation_listener{},
        contact_listener{},
        broad_phase_layer_interface{ _physicsConfig.broadPhaseLayerInterface },
        object_vs_broadphase_layer_filter{ _physicsConfig.objectVsBroadPhaseLayerFilter },
        object_vs_object_layer_filter{ _physicsConfig.objectLayerPairFilter },
        temp_allocator(10 * 1024 * 1024),
        job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1)
    {
        physics_system.Init(cMaxBodies,
                            cNumBodyMutexes,
                            cMaxBodyPairs,
                            cMaxContactConstraints,
                            *broad_phase_layer_interface,
                            *object_vs_broadphase_layer_filter,
                            *object_vs_object_layer_filter);
        physics_system.SetBodyActivationListener(&body_activation_listener);
        physics_system.SetContactListener(&contact_listener);
        body_interface = &physics_system.GetBodyInterface();

        physics_system.OptimizeBroadPhase();

#ifdef FT_DEBUG
        _debugRenderer = new JoltRenderingPipeline();
#endif

        FT_ENGINE_INFO("Physics instance created.");
    }

    Physics::~Physics()
    {
#ifdef FT_DEBUG
        delete _debugRenderer;
        _debugRenderer = nullptr;
#endif
    }

    void Physics::UpdatePhysics(float fixedDeltaTime)
    {
        physics_system.Update(1.0f / 60.0f, cCollisionSteps, &temp_allocator, &job_system);
    }

#ifdef FT_DEBUG
    void Physics::DrawDebug()
    {
        FT_ENGINE_ASSERT(Get()._debugRenderer != nullptr, "Debug renderer is null!");
        JPH::BodyManager::DrawSettings bodyDrawSettings;
        Get().physics_system.DrawBodies(bodyDrawSettings, Get()._debugRenderer);
    }
#endif

    JPH::DebugRenderer* Physics::GetDebugRenderer()
    {
#ifdef FT_DEBUG
        return Get()._debugRenderer;
#else
        return nullptr;
#endif
    }

    void Physics::SetLayerNames(const std::vector<PhysicsLayerInfo>& layers)
    {
        _layerNames = layers;
    }

    const std::vector<PhysicsLayerInfo>& Physics::GetLayerNames()
    {
        return _layerNames;
    }

    void Physics::AddConstraint(JPH::Constraint* inConstraint)
    {
        Get().physics_system.AddConstraint(inConstraint);
    }

    void Physics::AddConstraints(JPH::Constraint** inConstraints, int inNumber)
    {
        Get().physics_system.AddConstraints(inConstraints, inNumber);
    }

    void Physics::AddStepListener(JPH::PhysicsStepListener* inListener)
    {
        Get().physics_system.AddStepListener(inListener);
    }

    JPH::Body* Physics::CreateBody(const JPH::BodyCreationSettings& inSettings)
    {
        return Get().body_interface->CreateBody(inSettings);
    }

    void Physics::AddBody(const JPH::BodyID& inBodyID, JPH::EActivation inActivationMode)
    {
        return Get().body_interface->AddBody(inBodyID, inActivationMode);
    }

    JPH::BodyID Physics::CreateAndAddBody(JPH::BodyCreationSettings& inSettings,
                                          const entt::entity& rigidBodyId,
                                          const JPH::EActivation& inActivationMode)
    {
        inSettings.mUserData = static_cast<uint64>(rigidBodyId);
        auto body = CreateBody(inSettings);
        AddBody(body->GetID(), inActivationMode);
        return body->GetID();
    }

    JPH::Vec3 Physics::GetGravity()
    {
        return Get().physics_system.GetGravity();
    }

    void Physics::ActivateBody(const JPH::BodyID& inBodyID)
    {
        Get().body_interface->ActivateBody(inBodyID);
    }

    void Physics::RemoveAndDestroyBody(const JPH::BodyID& inBodyID)
    {
        if (!inBodyID.IsInvalid() && Get().body_interface->IsAdded(inBodyID))
        {
            Get().body_interface->RemoveBody(inBodyID);
            Get().body_interface->DestroyBody(inBodyID);
        }
    }

    JPH::BodyInterface& Physics::GetBodyInterface()
    {
        return *Get().body_interface;
    }

    const JPH::BodyLockInterfaceLocking& Physics::GetBodyLockInterface()
    {
        return Get().physics_system.GetBodyLockInterface();
    }

    entt::entity Physics::GetEntityID(const JPH::BodyID& inBodyID)
    {
        return static_cast<entt::entity>(GetBodyInterface().GetUserData(inBodyID));
    }

    void Physics::TraceImpl(const char* inFMT, ...)
    {
        // Format the message
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        // Print to the TTY
        std::cout << buffer << std::endl;
    }

#ifdef JPH_ENABLE_ASSERTS

    bool Physics::AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
    {
        // Print to the TTY
        std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "")
                  << std::endl;

        return true;
    }
#endif

    bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1,
                                                          JPH::BroadPhaseLayer inLayer2) const
    {
        switch (inLayer1)
        {
            case PhysicLayers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case PhysicLayers::PLAYER_MOVING:
                return true;
                /*case PhysicLayers::GHOST_MOVING:
                        return inLayer2 == BroadPhaseLayers::NON_MOVING;*/
            default:
                // JPH_ASSERT(false);
                return false;
        }
    }

    BPLayerInterfaceImpl::BPLayerInterfaceImpl()
    {
        // Create a mapping table from object to broad phase layer
        mObjectToBroadPhase[PhysicLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[PhysicLayers::PLAYER_MOVING] = BroadPhaseLayers::MOVING;
        mObjectToBroadPhase[PhysicLayers::SENSOR] = BroadPhaseLayers::NON_MOVING;

        for (int i = 0; i < PhysicLayers::NUM_LAYERS; ++i)
        {
            mObjectToBroadPhase[i] = BroadPhaseLayers::NON_MOVING;
        }
    }

    JPH::uint BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
    {
        return BroadPhaseLayers::NUM_LAYERS;
    }

    JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
    {
        JPH_ASSERT(inLayer < PhysicLayers::NUM_LAYERS);
        return mObjectToBroadPhase[inLayer];
    }

    const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
    {
        switch ((JPH::BroadPhaseLayer::Type)inLayer)
        {
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
                return "NON_MOVING";
            case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
                return "MOVING";
            default:
                // JPH_ASSERT(false);
                return "INVALID";
        }
    }

    bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
    {
        switch (inObject1)
        {
            case PhysicLayers::NON_MOVING:
                return inObject2 == PhysicLayers::PLAYER_MOVING;
            case PhysicLayers::PLAYER_MOVING:
                return inObject2 == PhysicLayers::NON_MOVING;
            default:
                // JPH_ASSERT(false);
                return false;
        }
    }
} // namespace Frost