#pragma once

#include "Frost.h"

#include <string>

namespace Editor
{
    class ProjectCloseEvent : public Frost::Event
    {
    public:
        ProjectCloseEvent() = default;

        Frost::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetCustomEventTypeID() const override { return "ProjectClose"; }
        static const char* GetStaticCustomEventTypeID() { return "ProjectClose"; }
        std::string ToString() const override { return "ProjectCloseEvent"; }

        static constexpr Frost::EventType GetStaticType() { return Frost::EventType::AppCustom; }
    };
} // namespace Editor