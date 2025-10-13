#pragma once

#include "Frost/Scene/ECS/ECS.h"

namespace Frost
{
	class System
	{
	public:
		virtual ~System() = default;

		virtual void OnAttach(ECS& ecs) {};
		virtual void OnDetach(ECS& ecs) {};
		virtual void Update(ECS& ecs, float deltaTime) {};
		virtual void LateUpdate(ECS& ecs, float deltaTime) {};
		virtual void FixedUpdate(ECS& ecs, float deltaTime) {};
	};
}
