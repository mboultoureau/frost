#include "Frost/Physics/Physics.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Frost/Scene/Systems/PhysicSystem.h>

#include <iostream>
#include <stdarg.h>
#include <cassert>

#include <Frost/Scene/Components/GameObjectInfo.h>
#include <Frost/Scene/Systems/PhysicSystem.h>


using namespace JPH;
namespace Frost
{
	Physics::Physics() :
		cMaxBodies(1024),
		cMaxBodyPairs(1024),
		cNumBodyMutexes(0),
		cMaxContactConstraints(1024),
		cDeltaTime(1.0f / 60.0f),
		body_activation_listener{},
		contact_listener{},
		broad_phase_layer_interface{ _physicsConfig.broadPhaseLayerInterface },
		object_vs_broadphase_layer_filter{ _physicsConfig.objectVsBroadPhaseLayerFilter },
		object_vs_object_layer_filter{ _physicsConfig.objectLayerPairFilter },
		temp_allocator(10 * 1024 * 1024),
		job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1),
		mapJBodyGameObject{}
	{
		physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);
		physics_system.SetBodyActivationListener(&body_activation_listener);

		physics_system.SetContactListener(&contact_listener);
		body_interface = &physics_system.GetBodyInterface();

		physics_system.OptimizeBroadPhase();
		std::cout << "Physics initialized" << std::endl;
	}

	Physics& Physics::Get()
	{
		static Physics _singleton;
		return _singleton;
	}


	Physics::~Physics()
	{
		JPH::UnregisterTypes();
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

	JPH::Vec3 Physics::GetGravity()
	{
		return Get().physics_system.GetGravity();
	}

	void Physics::ActivateBody(const JPH::BodyID& inBodyID)
	{
		Get().body_interface->ActivateBody(inBodyID);
	}

	void Physics::RemoveBody(const JPH::BodyID& inBodyID)
	{
		Get().body_interface->RemoveBody(inBodyID);
	}


	Frost::Transform::Vector3 Frost::Physics::JoltVectorToVector3(RVec3 v)
	{
		return { v.GetX(), v.GetY(), v.GetZ() };
	}

	Vec3 Frost::Physics::Vector3ToJoltVector(Transform::Vector3 v)
	{
		return { v.x, v.y, v.z };
	}

	void Physics::UpdatePhysics()
	{
		physics_system.Update(cDeltaTime, cCollisionSteps, &temp_allocator, &job_system);
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

	// Callback for asserts, connect this to your own assert handler if you have one
	bool Physics::AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
	{
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

		// Breakpoint
		return true;
	}

	void Physics::InitPhysics(PhysicsConfig& config, bool useConfig)
	{
		FT_ENGINE_ASSERT(!_physicsInitialized, "Physics has already been initialized!");
		FT_ENGINE_INFO("Initializing Physics...");

		if (useConfig)
		{
			FT_ENGINE_ASSERT(config.broadPhaseLayerInterface != nullptr, "broadPhaseLayerInterface in PhysicsConfig is null!");
			FT_ENGINE_ASSERT(config.objectLayerPairFilter != nullptr, "objectLayerPairFilter in PhysicsConfig is null!");
			FT_ENGINE_ASSERT(config.objectVsBroadPhaseLayerFilter != nullptr, "objectVsBroadPhaseLayerFilter in PhysicsConfig is null!");
			_physicsConfig = config;
		}
		else
		{
			_physicsConfig = PhysicsConfig {
				new BPLayerInterfaceImpl{},
				new ObjectLayerPairFilterImpl{},
				new ObjectVsBroadPhaseLayerFilterImpl{}
			};
		}

		JPH::RegisterDefaultAllocator();
		JPH::Trace = Physics::TraceImpl;
		JPH_IF_ENABLE_ASSERTS(AssertFailed = Physics::AssertFailedImpl;)
			JPH::Factory::sInstance = new JPH::Factory();
		RegisterTypes();

		Physics::Get();

		_physicsInitialized = true;
	}
	;

#endif 
	Frost::GameObject::Id Frost::Physics::GetGoIdFromJoltBodyId(JPH::BodyID id)
	{
		return mapJBodyGameObject.at(id);
	}

	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
	{
		switch (inObject1)
		{
		case PhysicLayers::NON_MOVING:
			return inObject2 == PhysicLayers::PLAYER_MOVING; // Non moving only collides with moving
		case PhysicLayers::PLAYER_MOVING:
			return inObject2 == PhysicLayers::NON_MOVING; // Non moving only collides with moving
			/*case Layers::SENSOR:
				return inObject2 == Layers::GHOST_MOVING;
			case Layers::GHOST_MOVING:
				return inObject2 == Layers::SENSOR;*/
		default:
			JPH_ASSERT(false);
			return false;
		}
	}


	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
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
			JPH_ASSERT(false);
			return false;
		}
	}

	BPLayerInterfaceImpl::BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[PhysicLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[PhysicLayers::PLAYER_MOVING] = BroadPhaseLayers::MOVING;
		mObjectToBroadPhase[PhysicLayers::SENSOR] = BroadPhaseLayers::NON_MOVING;
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
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		default:													JPH_ASSERT(false); return "INVALID";
		}
	}

	int Physics::SetShapeToRigidbody(GameObject::Id id, JPH::ShapeRefC shapeRef) {
		_rigidbodyFuturColliders.insert({ id, shapeRef });
		return shapeRegistry.registerShape(shapeRef);
	}


	JPH::ShapeRefC& Physics::GetRigidbodyShapeRef(RigidBody* rb) {
		return shapeRegistry.getRef(rb->shapeId);
	}
}