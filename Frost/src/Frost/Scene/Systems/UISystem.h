#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Pipeline/HUDRenderingPipeline.h"

namespace Frost
{
	class UISystem : public System
	{
	public:
		UISystem();
		void Update(Frost::ECS& ecs, float deltaTime) override;
		void LateUpdate(Frost::ECS& ecs, float deltaTime) override;

	private:
		HUDRenderingPipeline _pipeline;
	};
}
