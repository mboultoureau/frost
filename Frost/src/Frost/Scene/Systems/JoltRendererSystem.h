#pragma once

#include "Frost/Scene/ECS/System.h"

namespace Frost
{
	class JoltRendererSystem : public System
	{
	public:
		JoltRendererSystem();
		virtual void LateUpdate(ECS& ecs, float deltaTime) override;
	};
}