#pragma once

#include <Jolt/Physics/Body/Body.h>

namespace Frost
{
	struct RigidBody2
	{
		RigidBody2(JPH::Body* body) : body{ body } {};

		JPH::Body* body;
	};
}