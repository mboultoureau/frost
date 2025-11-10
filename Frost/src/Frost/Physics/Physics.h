#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/ECS/ECS.h"
#include "Frost/Scene/Components/Transform.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>

#include "Frost/Physics/Layers.h"
#include "Frost/Physics/PhysicListener.h"
#include "Frost/Physics/ShapeRegistry.h"
#include "Frost/Physics/DebugRendererPhysics.h"
#include "Frost/Physics/PhysicsConfig.h"

#include <vector>
#include <map>
#include <Frost/Renderer/DX11/TextureDX11.h>


namespace Frost
{
	class Physics : NoCopy
	{
		friend class PhysicSystem;
	public:
		using PairIdCollisionVector = std::vector<std::pair<Frost::GameObject::Id, Frost::GameObject::Id>>;

		static Physics& Get();

		Physics();
		~Physics();

#ifdef FT_DEBUG
		static void DrawDebug();
		static Frost::DebugRendererPhysicsConfig& GetDebugRendererConfig();
		static void SetDebugRendererConfig(Frost::DebugRendererPhysicsConfig config);
#endif

		static void AddConstraint(JPH::Constraint* inConstraint);
		static void AddConstraints(JPH::Constraint** inConstraints, int inNumber);
		static void AddStepListener(JPH::PhysicsStepListener* inListener);
		static JPH::Body* CreateBody(const JPH::BodyCreationSettings& inSettings);
		static void AddBody(const JPH::BodyID& inBodyID, JPH::EActivation inActivationMode);
		static JPH::BodyID CreateAndAddBody(JPH::BodyCreationSettings& inSettings, const GameObject::Id& rigidBodyId, const JPH::EActivation& inActivationMode);
		static JPH::Vec3 GetGravity();
		static void ActivateBody(const JPH::BodyID& inBodyID);
		static void RemoveBody(const JPH::BodyID& inBodyID);


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
		JPH::BroadPhaseLayerInterface* broad_phase_layer_interface;

		static void TraceImpl(const char* inFMT, ...);
#ifdef JPH_ENABLE_ASSERTS
		static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine);
#endif 

		static bool HasPhysicsBeenInitialized;
		static void InitPhysics(PhysicsConfig& config, bool useConfig);

		Frost::GameObject::Id GetGoIdFromJoltBodyId(JPH::BodyID id);
		std::unordered_map<JPH::BodyID, Frost::GameObject::Id> mapJBodyGameObject;


		JPH::ShapeRefC CreateHeightFieldShapeFromTexture(TextureDX11* heightTexture, TextureChannel channel, float heightScale, Transform::Vector3 transformScale);

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

		JPH::ObjectVsBroadPhaseLayerFilter* object_vs_broadphase_layer_filter;
		JPH::ObjectLayerPairFilter* object_vs_object_layer_filter;

		const float cDeltaTime;

		inline static bool _physicsInitialized = false;
		inline static PhysicsConfig _physicsConfig;

#ifdef FT_DEBUG
		JPH::DebugRenderer* _debugRenderer;
		Frost::DebugRendererPhysicsConfig _debugRendererConfig;
#endif
	};
}	// NAMESPACE FROST
