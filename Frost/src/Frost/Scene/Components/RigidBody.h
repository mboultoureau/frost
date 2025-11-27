#pragma once
#include "Frost/Debugging/Logger.h"

#include "Frost/Physics/Physics.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/EActivation.h>

namespace Frost::Component
{

    struct PhysicBody
    {
        PhysicBody(JPH::BodyCreationSettings& bodySettings, GameObject::Id id, JPH::EActivation activationMode) :
            bodyId{ Physics::CreateAndAddBody(bodySettings, id, activationMode) } {};

        PhysicBody(JPH::BodyID bodyId, GameObject::Id id) : bodyId{ bodyId }
        {
            Physics::Get().body_interface->SetUserData(bodyId, static_cast<JPH::uint64>(id));
        };

        ~PhysicBody() { Physics::RemoveAndDestroyBody(bodyId); }

        JPH::BodyID bodyId;
    };

    struct RigidBody
    {
        RigidBody(JPH::BodyCreationSettings& bodySettings, GameObject::Id id, JPH::EActivation activationMode) :
            physicBody{ std::make_shared<PhysicBody>(bodySettings, id, activationMode) } {};

        RigidBody(JPH::BodyID bodyId, GameObject::Id id) : physicBody{ std::make_shared<PhysicBody>(bodyId, id) } {};

        std::shared_ptr<PhysicBody> physicBody;
    };

} // namespace Frost::Component