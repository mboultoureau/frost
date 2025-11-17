#pragma once

#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Scene/ECS/System.h"

namespace Frost
{
	class ModelRendererSystem : public System
	{
	public:
		ModelRendererSystem();
		void LateUpdate(Frost::ECS& ecs, float deltaTime) override;

	private:
		DeferredRenderingPipeline _deferredRendering;
	};
}


