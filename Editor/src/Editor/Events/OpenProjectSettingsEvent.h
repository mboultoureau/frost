#pragma once

#include "Frost.h"

#include <string>

namespace Editor
{
    class OpenProjectSettingsEvent : public Frost::Event
    {
    public:
        OpenProjectSettingsEvent() = default;

        Frost::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetCustomEventTypeID() const override { return "OpenProjectSettings"; }
        static const char* GetStaticCustomEventTypeID() { return "OpenProjectSettings"; }
        std::string ToString() const override { return "OpenProjectSettingsEvent"; }

        static constexpr Frost::EventType GetStaticType() { return Frost::EventType::AppCustom; }
    };
} // namespace Editor