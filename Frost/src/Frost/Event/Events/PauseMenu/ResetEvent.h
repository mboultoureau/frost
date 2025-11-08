#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
	class ResetEvent : public Event
	{
	public:
		ResetEvent() = default;

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "ResetEvent"; }

		static EventType GetStaticType() { return EventType::GameReset; }
	};
}