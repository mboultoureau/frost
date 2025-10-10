#pragma once

#include "Frost/Scene/ECS/Component.h"

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
		float perspectiveFOV = 60.0f;
		float orthographicSize = 10.0f;
		float nearClip = 0.1f;
		float farClip = 1000.0f;

		float backgroundColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		float viewportX = 0.0f;
		float viewportY = 0.0f;
		float viewportWidth = 1.0f;
		float viewportHeight = 1.0f;
	};
}