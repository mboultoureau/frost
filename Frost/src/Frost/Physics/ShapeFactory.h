#pragma once

#include "Frost/Asset/HeightMapModel.h"
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace Frost
{
	class ShapeFactory
	{
	public:
		static JPH::ShapeRefC CreateHeightMap(const HeightMapConfig& config);
	};
}