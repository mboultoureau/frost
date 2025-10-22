#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
	class KeyPressedEvent : public Event
	{
	public:
		KeyPressedEvent() = default;

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "KeyPressedEvent"; }

		static EventType GetStaticType() { return EventType::KeyPressed; }
	};
}
