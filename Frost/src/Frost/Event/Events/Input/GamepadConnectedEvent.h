#pragma once

#include "Frost/Event/Event.h"
#include "Frost.h"

namespace Frost
{
	class GamepadConnectedEvent : public Event
	{
	public:
		GamepadConnectedEvent(Gamepad::GamepadId id) : _id{ id } {}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "GamepadConnectedEvent"; }

		static EventType GetStaticType() { return EventType::GamepadConnected; }

		Gamepad::GamepadId GetGamepadId() const noexcept { return _id; }

	private:
		Gamepad::GamepadId _id;
	};
}
