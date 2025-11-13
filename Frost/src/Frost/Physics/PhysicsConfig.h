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

	/// Allows objects from a specific broad phase layer only
	class SpecifiedBroadPhaseLayerFilter : public JPH::BroadPhaseLayerFilter
	{
	public:
		/// Constructor
		explicit SpecifiedBroadPhaseLayerFilter(JPH::BroadPhaseLayer inLayer) :
			mLayer(inLayer)
		{
		}

		// See BroadPhaseLayerFilter::ShouldCollide
		virtual bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
		{
			return mLayer == inLayer;
		}

	private:
		JPH::BroadPhaseLayer mLayer;
	};


	/// Allows objects from a specific layer only
	class SpecifiedObjectLayerFilter : public JPH::ObjectLayerFilter
	{
	public:
		explicit SpecifiedObjectLayerFilter(JPH::ObjectLayer inLayer) :
			mLayer(inLayer)
		{
		}
		virtual bool ShouldCollide(JPH::ObjectLayer inLayer) const override
		{
			return mLayer == inLayer;
		}

	private:
		JPH::ObjectLayer mLayer;
	};
}