#pragma once

#include "Frost.h"

#include <string>

namespace Editor
{
    class NewSceneEvent : public Frost::Event
    {
    public:
        NewSceneEvent() {}

        Frost::EventType GetEventType() const override { return GetStaticType(); }
        const char* GetCustomEventTypeID() const override { return "NewScene"; }
        static const char* GetStaticCustomEventTypeID() { return "NewScene"; }
        std::string ToString() const override { return "NewSceneEvent"; }

        static constexpr Frost::EventType GetStaticType() { return Frost::EventType::AppCustom; }
    };
} // namespace Editor