#pragma once

#include "Frost/Renderer/Viewport.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Utils/Math/Angle.h"

namespace Frost::Component
{
	struct Camera : public Component
	{
		enum ProjectionType
		{
			Perspective = 0,
			Orthographic = 1
		};

		ProjectionType projectionType = ProjectionType::Perspective;
		Math::Angle<Math::Radian> perspectiveFOV = Math::Angle<Math::Degree>(60.0f);
		float orthographicSize = 10.0f;
		float nearClip = 0.1f;
		float farClip = 1000.0f;

		bool frustumCulling = true;
		float frustumPadding = 10.0f;

		bool clearOnRender = true;
		Math::Color4 backgroundColor = { 0.1f, 0.1f, 0.1f, 1.0f };

		bool lookAtPositionEnabled = false;
		Math::Vector3 lookAtPosition = { 0.0f, 0.0f, 0.0f };

		Viewport viewport;
	};
}