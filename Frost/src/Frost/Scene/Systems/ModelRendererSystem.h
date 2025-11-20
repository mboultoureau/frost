#pragma once

#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Renderer/Pipeline/SkyboxPipeline.h"
#include "Frost/Scene/ECS/System.h"
#include "Frost/Asset/Texture.h"

#include <memory>

namespace Frost
{
	class ModelRendererSystem : public System
	{
	public:
		ModelRendererSystem();
		void LateUpdate(Frost::ECS& ecs, float deltaTime) override;

	private:
		DeferredRenderingPipeline _deferredRendering;
		SkyboxPipeline _skyboxPipeline;

		std::unique_ptr<Texture> _source;
		std::unique_ptr<Texture> _destination;
	};
}


