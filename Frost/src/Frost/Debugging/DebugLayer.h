#pragma once

#include "Frost/Core/Layer.h"
#include "Frost/Scene/Scene.h"

#include <vector>

namespace Frost
{
	class DebugLayer : public Layer
	{
	public:
		DebugLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float deltaTime) override;
		void OnLateUpdate(float deltaTime) override;

		void AddScene(Scene* scene) { _scenes.push_back(scene); }

		static Layer::LayerName GetStaticName() { return "DebugLayer"; }

	private:
		std::vector<Scene*> _scenes;
	};
}
