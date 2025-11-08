#pragma once

namespace Frost
{
	enum class EventType
	{
		None = 0,
		WindowClose,
		WindowResize,
		DebugOptionChanged,
		KeyPressed,
		KeyReleased,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled,
		GamepadButtonPressed,
		GamepadButtonReleased,
		GamepadAxisMoved,
		GamepadConnected,
		GamepadDisconnected,
		GamePaused,
		GameUnpaused,
		GameReset,
		_COUNT
	};
}