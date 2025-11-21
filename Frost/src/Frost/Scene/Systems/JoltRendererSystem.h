#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/WorldTransform.h"

namespace Frost
{
	class JoltRendererSystem : public System
	{
	public:
		JoltRendererSystem();
		virtual void LateUpdate(Scene& scene, float deltaTime) override;
	};
}