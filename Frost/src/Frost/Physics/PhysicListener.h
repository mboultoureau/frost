#pragma once
#include "Frost/Scene/ECS/GameObject.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Body/Body.h>

// === Listeners ========================================================

namespace Frost
{
	struct BodyActivationParameters {
		const JPH::BodyID& inBodyID;
		JPH::uint64 inBodyUserData;
	};

	struct BodyOnContactParameters {
		const JPH::Body& inBody1;
		const JPH::Body& inBody2;
		const JPH::ContactManifold& inManifold;
		JPH::ContactSettings& ioSettings;
	};


	class MyBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		void OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;

		void OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override;
	};

	class MyContactListener : public JPH::ContactListener
	{
	public:
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
			const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
			const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
	};
}
