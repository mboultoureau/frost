#pragma once

#include "Frost/Scene/Scene.h"
#include "Frost/Scene/ECS/System.h"

namespace Frost
{
	class ScriptableSystem : public System
	{
	public:
		void Update(Scene& scene, float deltaTime) override;
		void FixedUpdate(Scene& scene, float fixedDeltaTime) override;
		void LateUpdate(Scene& scene, float deltaTime) override;
	};
}
