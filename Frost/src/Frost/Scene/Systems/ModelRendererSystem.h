#pragma once

#include "Frost/Renderer/Pipeline/DeferredRenderingPipeline.h"
#include "Frost/Renderer/Pipeline/SkyboxPipeline.h"
#include "Frost/Scene/ECS/System.h"
#include "Frost/Asset/Texture.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Light.h"

#include <memory>

namespace Frost
{
	class ModelRendererSystem : public System
	{
	public:
		ModelRendererSystem();
		void LateUpdate(Scene& scene, float deltaTime) override;

	private:
		DeferredRenderingPipeline _deferredRendering;
		SkyboxPipeline _skyboxPipeline;

		std::unique_ptr<Texture> _source;
		std::unique_ptr<Texture> _destination;
	};
}


