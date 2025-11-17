#pragma once

#include "Frost/Utils/Math/Vector.h"
#include "Frost/Utils/Math/Angle.h"

namespace Frost::Component
{
	enum class LightType : uint8_t
	{
		Directional,
		Point,
		Spot
	};

	struct Light
	{
		LightType type = LightType::Point;
		Math::Color3 color = { 1.0f, 1.0f, 1.0f };

		float intensity = 1.0f;
		float radius = 10.0f;
		Math::Angle<Math::Radian> innerConeAngle = Math::Angle<Math::Degree>(15.0f);
		Math::Angle<Math::Radian> outerConeAngle = Math::Angle<Math::Degree>(25.0f);
	};
}