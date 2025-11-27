#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
    class UnPauseEvent : public Event
    {
    public:
        UnPauseEvent() = default;

        EventType GetEventType() const override { return GetStaticType(); }
        std::string ToString() const override { return "UnPauseEvent"; }

        static EventType GetStaticType() { return EventType::GameUnpaused; }
    };
} // namespace Frost