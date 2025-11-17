#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Input/Devices/Keyboard.h"

namespace Frost
{
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent(VirtualKeyCode keyCode) : _keyCode{ keyCode } {}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "KeyPressedEvent"; }

		static EventType GetStaticType() { return EventType::KeyPressed; }

		VirtualKeyCode GetKeyCode() const noexcept { return _keyCode; }

	private:
		VirtualKeyCode _keyCode;
	};
}
