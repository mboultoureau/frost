#pragma once

#include "Frost/Event/Event.h"

#include <string>

namespace Frost
{
    class LoadLevelEvent : public Event
    {
    public:
        LoadLevelEvent(const std::string& levelName) : _levelName(levelName) {}

        EventType GetEventType() const override { return GetStaticType(); }
        std::string ToString() const override { return "LoadLevel"; }

        static constexpr EventType GetStaticType() { return EventType::LoadLevel; }

        const std::string& GetLevelName() const noexcept { return _levelName; }

    private:
        std::string _levelName;
    };
} // namespace Frost