#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Renderer/Viewport.h"

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

		bool clearOnRender = true;
		float backgroundColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

		Viewport viewport;
	};
}