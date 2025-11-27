#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;

        EventType GetEventType() const override { return GetStaticType(); }
        std::string ToString() const override { return "WindowCloseEvent"; }

        static EventType GetStaticType() { return EventType::WindowClose; }
    };
} // namespace Frost
