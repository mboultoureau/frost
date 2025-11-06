#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost.h"

namespace Frost
{
	struct Camera : public Component
	{
		enum ProjectionType
		{
			Perspective = 0,
			Orthographic = 1
		};

		ProjectionType projectionType = ProjectionType::Perspective;
		Angle<Degree> perspectiveFOV = 60.0_deg;
		float orthographicSize = 10.0f;
		float nearClip = 0.1f;
		float farClip = 1000.0f;

		bool frustumCulling = true;
		float frustumPadding = 10.0f;

		bool clearOnRender = true;
		float backgroundColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		bool lookAtPositionEnabled = false;
		Transform::Vector3 lookAtPosition = { 0.0f, 0.0f, 0.0f };

		Viewport viewport;
	};
}