#pragma once

#include <Jolt/Physics/Body/Body.h>

namespace Frost
{
	struct RigidBody
	{
		//RigidBody(JPH::Body* body) : body{ body } {};
		RigidBody(JPH::BodyCreationSettings& bodySettings, GameObject::Id id, JPH::EActivation activationMode) : bodyId{ Physics::CreateAndAddBody(bodySettings, id, activationMode) } {};

		JPH::BodyID bodyId;
	};
}