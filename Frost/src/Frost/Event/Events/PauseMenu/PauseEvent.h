#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
    class PauseEvent : public Event
    {
    public:
        PauseEvent() = default;

        EventType GetEventType() const override { return GetStaticType(); }
        std::string ToString() const override { return "PauseEvent"; }

        static EventType GetStaticType() { return EventType::GamePaused; }
    };
} // namespace Frost