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
		void OnFixedUpdate(float fixedDeltaTime) override;

		void AddScene(Scene* scene) { _scenes.push_back(scene); }

		static Layer::LayerName GetStaticName() { return "DebugLayer"; }

	private:
		std::vector<Scene*> _scenes;

		static constexpr int FRAME_TIME_HISTORY_SIZE = 100; // Number of frames to track
		float m_FrameTimes[FRAME_TIME_HISTORY_SIZE] = {};
		int m_FrameTimeHistoryIndex = 0;
		float m_MaxFrameTime = 0.0f;

		// Physics Time Graph Data (NEW)
		float m_FixedUpdateTimes[FRAME_TIME_HISTORY_SIZE] = {};
		int m_FixedUpdateTimeHistoryIndex = 0;
		float m_MaxFixedUpdateTime = 0.0f;

		// ... existing private methods
		void _DrawPerformancePanel(); // New private method for the graph
		void _DrawRenderingOptionsPanel();
		void _DrawSceneHierarchyPanel();
	};
}
