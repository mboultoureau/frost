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
	//	void OnFixedUpdate(float fixedDeltaTime) override;

		void AddScene(Scene* scene);
		void RemoveScene(Scene* scene);

		static Layer::LayerName GetStaticName() { return "DebugLayer"; }

	private:
		bool _displayDebug = true;
		std::vector<std::unique_ptr<DebugPanel>> _debugPanels;

	//private:

	//	// Render update graph
	//	static constexpr int FRAME_TIME_HISTORY_SIZE = 100;
	//	float _frameTimes[FRAME_TIME_HISTORY_SIZE] = {};
	//	int _frameTimeHistoryIndex = 0;
	//	float _maxFrameTime = 0.0f;

	//	// Physics update graph
	//	float _fixedUpdateTimes[FRAME_TIME_HISTORY_SIZE] = {};
	//	int _fixedUpdateTimeHistoryIndex = 0;
	//	float _maxFixedUpdateTime = 0.0f;

	//	struct LogEntry
	//	{
	//		float timestamp;
	//		std::string message;
	//		EventType type;
	//	};

	//	static constexpr size_t MAX_LOG_ENTRIES = 100;
	//	std::deque<LogEntry> _eventLog;
	//	Timer _logTimer;

	//	void _LogEvent(const Event& e, const std::string& message);

	//	bool _OnDebugOptionChanged(DebugOptionChangedEvent& e);
	//	bool _OnWindowClose(WindowCloseEvent& e);
	//	bool _OnWindowResize(WindowResizeEvent& e);
	//	bool _OnGamepadConnected(GamepadConnectedEvent& e);
	//	bool _OnGamepadDisconnected(GamepadDisconnectedEvent& e);

	//	void _DrawEventLogPanel();
	//	void _DrawInputPanel();
	//	void _DrawPerformancePanel();
	//	void _DrawSceneHierarchyPanel();
	//	void _DrawGamepadPanel(Gamepad& gamepad);
	//	void _DrawMousePanel();
	//	void _DrawKeyboardPanel();
	//	void _DrawJoystickVisual(const char* label, const Frost::Gamepad::Joystick& joy, float radius);
	//	bool _DrawTransformControl(const char* label, Frost::Gamepad::Transform& currentTransform);
	};
}
