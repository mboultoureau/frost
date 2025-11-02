#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/ECS/ECS.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>

#include <vector>

#include "Frost/Physics/Layers.h"
#include "Frost/Physics/PhysicListener.h"
#include <map>
#include "Frost/Physics/ShapeRegistry.h"


namespace Frost
{
	class Physics : NoCopy
	{
		friend class PhysicSystem;
		friend class RigidBody;

	public:
		using PairIdCollisionVector = std::vector<std::pair<Frost::GameObject::Id, Frost::GameObject::Id>>;

		static Physics& Get();

		Physics();
		~Physics();


		static Frost::Transform::Vector3 JoltVectorToVector3(JPH::RVec3 v);
		static JPH::Vec3 Vector3ToJoltVector(Frost::Transform::Vector3 v);

		void UpdatePhysics();

		std::vector<Frost::GameObject::Id> bodiesOnAwake;
		std::vector<Frost::GameObject::Id> bodiesOnSleep;
		PairIdCollisionVector bodiesOnCollisionEnter;
		PairIdCollisionVector bodiesOnCollisionStay;
		PairIdCollisionVector bodiesOnCollisionExit;

		JPH::PhysicsSystem physics_system;
		JPH::BodyInterface* body_interface;
		
		Frost::MyBodyActivationListener body_activation_listener;
		Frost::MyContactListener contact_listener;
		Frost::BPLayerInterfaceImpl broad_phase_layer_interface;

		static void TraceImpl(const char* inFMT, ...);
#ifdef JPH_ENABLE_ASSERTS
		static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine);
#endif 

		static bool HasPhysicsBeenInitialized;
		static void InitPhysics();

		Frost::GameObject::Id GetGoIdFromJoltBodyId(JPH::BodyID id);
		std::unordered_map<JPH::BodyID, Frost::GameObject::Id> mapJBodyGameObject;


		// adds shape to body associated to rigidBody, then returns subShape index in its mutableCompoundShape
		//int AddSubShapeToBody(JPH::BodyID&, JPH::ShapeRefC&, JPH::EActivation, JPH::Vec3Arg& localPosition, JPH::QuatArg& localRotation);
		//void RemoveSubShapeFromBody(JPH::BodyID&, JPH::uint32 subShapeInternalIndex, JPH::EActivation);
		
		// WARNING : this MUST be called the same frame you create a rigidbody
		JPH::ShapeRefC& GetRigidbodyShapeRef(Frost::RigidBody* rb);


	private:
		int SetShapeToRigidbody(GameObject::Id id, JPH::ShapeRefC shapeRef);

		ShapeRegistry shapeRegistry;
		std::unordered_map<GameObject::Id, JPH::ShapeRefC> _rigidbodyFuturColliders;

		JPH::TempAllocatorImpl temp_allocator;
		JPH::JobSystemThreadPool job_system;

		const int cCollisionSteps = 1;
		const JPH::uint cMaxBodies;
		const JPH::uint cNumBodyMutexes;
		const JPH::uint cMaxBodyPairs;
		const JPH::uint cMaxContactConstraints;

		Frost::ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
		Frost::ObjectLayerPairFilterImpl object_vs_object_layer_filter;

		const float cDeltaTime;
	};
}	// NAMESPACE FROST
