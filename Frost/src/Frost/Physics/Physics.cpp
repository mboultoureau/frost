#include "Frost/Physics/Physics.h"

#include <Jolt/RegisterTypes.h>

namespace Frost
{
	Physics::Physics()
	{
		JPH::RegisterDefaultAllocator();
	}

	Physics::~Physics()
	{
		JPH::UnregisterTypes();
	}
}