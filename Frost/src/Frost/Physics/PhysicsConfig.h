#pragma once

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace Frost
{
	struct PhysicsConfig
	{
		JPH::BroadPhaseLayerInterface* broadPhaseLayerInterface = nullptr;
		JPH::ObjectLayerPairFilter* objectLayerPairFilter = nullptr;
		JPH::ObjectVsBroadPhaseLayerFilter* objectVsBroadPhaseLayerFilter = nullptr;
	};
}