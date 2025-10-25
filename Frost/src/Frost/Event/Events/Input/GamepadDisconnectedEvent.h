#pragma once

#include "Frost/Event/Event.h"
#include "Frost.h"

namespace Frost
{
	class GamepadDisconnectedEvent : public Event
	{
	public:
		GamepadDisconnectedEvent(Gamepad::GamepadId id) : _id{ id } {}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "GamepadDisconnectedEvent"; }

		static EventType GetStaticType() { return EventType::GamepadDisconnected; }

		Gamepad::GamepadId GetGamepadId() const noexcept { return _id; }

	private:
		Gamepad::GamepadId _id;
	};
}
