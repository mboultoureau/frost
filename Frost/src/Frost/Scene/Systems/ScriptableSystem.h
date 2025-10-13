#pragma once

#include "Frost/Scene/ECS/System.h"

namespace Frost
{
	class ScriptableSystem : public System
	{
	public:
		void Update(Frost::ECS& ecs, float deltaTime) override;
		void FixedUpdate(Frost::ECS& ecs, float fixedDeltaTime) override;
		void LateUpdate(Frost::ECS& ecs, float deltaTime) override;
	};
}
