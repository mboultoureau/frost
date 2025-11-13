#include "PhysicListener.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include "Physics.h"


void Frost::MyBodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
	BodyActivationParameters params{ inBodyID, inBodyUserData };
	Physics::Get().bodiesOnAwake.push_back(params);
}

void Frost::MyBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
	BodyActivationParameters params{ inBodyID, inBodyUserData };
	Physics::Get().bodiesOnSleep.push_back(params);
}

void Frost::MyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	BodyOnContactParameters params{ inBody1, inBody2, inManifold, ioSettings };
	Physics::Get().bodiesOnCollisionEnter.emplace_back(params);
}

void Frost::MyContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	BodyOnContactParameters params{ inBody1, inBody2, inManifold, ioSettings };
	Physics::Get().bodiesOnCollisionStay.emplace_back(params);
}

void Frost::MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	/*// TODO: https://jrouwe.github.io/JoltPhysics/class_contact_listener.html#aef8f0cac1d560622743420afcd1ce17a
	// Faire une map entre BodyID et GameObjectId 
	auto& physics = Physics::Get();
	int userData1 = physics.GetObjectID(inSubShapePair.GetBody1ID());
	int userData2 = physics.GetObjectID(inSubShapePair.GetBody2ID());
	physics.bodiesOnCollisionExit.emplace_back(userData1, userData2);*/
}
