#include "PhysicListener.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/ContactListener.h>

#include "Physics.h"

void
Frost::MyBodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
    BodyActivationParameters params{ inBodyID, inBodyUserData };
    Physics::Get().mutexOnAwake.lock();
    Physics::Get().bodiesOnAwake.push_back(params);
    Physics::Get().mutexOnAwake.unlock();
}

void
Frost::MyBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData)
{
    BodyActivationParameters params{ inBodyID, inBodyUserData };
    Physics::Get().mutexOnSleep.lock();
    Physics::Get().bodiesOnSleep.push_back(params);
    Physics::Get().mutexOnSleep.unlock();
}

void
Frost::MyContactListener::OnContactAdded(const JPH::Body& inBody1,
                                         const JPH::Body& inBody2,
                                         const JPH::ContactManifold& inManifold,
                                         JPH::ContactSettings& ioSettings)
{
    BodyOnContactParameters params{ inBody1, inBody2, inManifold, ioSettings };
    Physics::Get().mutexOnCollisionEnter.lock();
    Physics::Get().bodiesOnCollisionEnter.emplace_back(params);
    Physics::Get().mutexOnCollisionEnter.unlock();
}

void
Frost::MyContactListener::OnContactPersisted(const JPH::Body& inBody1,
                                             const JPH::Body& inBody2,
                                             const JPH::ContactManifold& inManifold,
                                             JPH::ContactSettings& ioSettings)
{
    BodyOnContactParameters params{ inBody1, inBody2, inManifold, ioSettings };
    Physics::Get().mutexOnCollisionStay.lock();
    Physics::Get().bodiesOnCollisionStay.emplace_back(params);
    Physics::Get().mutexOnCollisionStay.unlock();
}
