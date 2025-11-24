#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"
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
		void OnFixedUpdate(float fixedDeltaTime) override;

		void AddScene(Scene* scene);
		void RemoveScene(Scene* scene);

		static Layer::LayerName GetStaticName() { return "DebugLayer"; }

	private:
		bool _displayDebug = false;
		std::vector<std::unique_ptr<DebugPanel>> _debugPanels;
	};
}
