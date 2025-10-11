#pragma once

#include "Frost/Scene/ECS/ECS.h"

namespace Frost
{
	class System
	{
	public:
		virtual ~System() = default;

		virtual void Update(ECS& ecs, float deltaTime) {};
		virtual void LateUpdate(ECS& ecs, float deltaTime) {};
		virtual void FixedUpdate(ECS& ecs, float deltaTime) {};
	};
}
