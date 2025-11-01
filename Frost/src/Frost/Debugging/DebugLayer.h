#pragma once

#include "Frost/Core/Layer.h"
#include "Frost/Scene/Scene.h"
#include "Frost/Core/Timer.h"
#include "Frost/Event/Events/Debugging/DebugOptionChangedEvent.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Event/Events/Input/GamepadConnectedEvent.h"
#include "Frost/Event/Events/Input/GamepadDisconnectedEvent.h"

#include "Frost.h"

#include <vector>
#include <deque>

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

		// Render update graph
		static constexpr int FRAME_TIME_HISTORY_SIZE = 100;
		float _frameTimes[FRAME_TIME_HISTORY_SIZE] = {};
		int _frameTimeHistoryIndex = 0;
		float _maxFrameTime = 0.0f;

		// Physics update graph
		float _fixedUpdateTimes[FRAME_TIME_HISTORY_SIZE] = {};
		int _fixedUpdateTimeHistoryIndex = 0;
		float _maxFixedUpdateTime = 0.0f;

		struct LogEntry
		{
			float timestamp;
			std::string message;
			EventType type;
		};

		static constexpr size_t MAX_LOG_ENTRIES = 100;
		std::deque<LogEntry> _eventLog;
		Timer _logTimer;

		void _LogEvent(const Event& e, const std::string& message);

		bool _OnDebugOptionChanged(DebugOptionChangedEvent& e);
		bool _OnWindowClose(WindowCloseEvent& e);
		bool _OnWindowResize(WindowResizeEvent& e);
		bool _OnGamepadConnected(GamepadConnectedEvent& e);
		bool _OnGamepadDisconnected(GamepadDisconnectedEvent& e);

		void _DrawEventLogPanel();
		void _DrawInputPanel();
		void _DrawPerformancePanel();
		void _DrawRenderingOptionsPanel();
		void _DrawSceneHierarchyPanel();
		void _DrawGamepadPanel(Gamepad& gamepad);
		void _DrawMousePanel();
		void _DrawKeyboardPanel();
		void _DrawJoystickVisual(const char* label, const Frost::Gamepad::Joystick& joy, float radius);
		bool _DrawTransformControl(const char* label, Frost::Gamepad::Transform& currentTransform);
	};
}
