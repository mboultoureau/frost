#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Utils/UUID.h"

#include <functional>
#include <string>

namespace Frost
{
    class EventHandlerInterface
    {
    public:
        using Id = UUID<EventHandlerInterface>;

        virtual ~EventHandlerInterface() = default;
        virtual bool OnEvent(Event& event) = 0;
        virtual Id GetID() const = 0;

    protected:
        EventHandlerInterface() = default;
    };

    template<typename EventType>
    class EventHandler : public EventHandlerInterface
    {
        static_assert(std::is_base_of<Event, EventType>::value, "EventType must inherit from Frost::Event");

    public:
        using EventCallback = std::function<bool(EventType&)>;

        EventHandler(const EventCallback& callback) : _callback(callback), _id(Id::generate()) {}

        virtual ~EventHandler() override = default;

        bool OnEvent(Event& event) override
        {
            if (event.GetEventType() == EventType::GetStaticType())
            {
                if constexpr (EventType::GetStaticType() == Frost::EventType::AppCustom)
                {
                    if (std::strcmp(event.GetCustomEventTypeID(), EventType::GetStaticCustomEventTypeID()) != 0)
                    {
                        return false;
                    }
                }

                if (_callback(static_cast<EventType&>(event)))
                {
                    event.Handle();
                    return true;
                }
            }
            return false;
        }

        Id GetID() const override { return _id; }

    private:
        EventCallback _callback;
        Id _id;
    };
} // namespace Frost