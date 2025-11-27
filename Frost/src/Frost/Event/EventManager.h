#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Event/EventHandler.h"
#include "Frost/Utils/NoCopy.h"

#include <memory>
#include <vector>

namespace Frost
{
    using EventHandlerId = EventHandlerInterface::Id;

    class EventManager : NoCopy
    {
    public:
        template<typename EventType>
        static EventHandlerId Subscribe(const typename EventHandler<EventType>::EventCallback& callback)
        {
            static_assert(std::is_base_of<Event, EventType>::value, "EventType must inherit from Frost::Event");

            auto handler = std::make_shared<EventHandler<EventType>>(callback);
            EventHandlerId handlerID = handler->GetID();
            auto& handlers = Get()._handlers[EventType::GetStaticType()];

            handlers.push_back(std::move(handler));

            return handlerID;
        }

        template<typename EventType>
        static EventHandlerId SubscribeFront(const typename EventHandler<EventType>::EventCallback& callback)
        {
            static_assert(std::is_base_of<Event, EventType>::value, "EventType must inherit from Frost::Event");

            auto handler = std::make_shared<EventHandler<EventType>>(callback);
            EventHandlerId handlerID = handler->GetID();
            auto& handlers = Get()._handlers[EventType::GetStaticType()];

            handlers.insert(handlers.begin(), std::move(handler));

            return handlerID;
        }

        template<typename EventType>
        static void Unsubscribe(EventHandlerId handlerID)
        {
            if (Get()._handlers[EventType::GetStaticType()].size() == 0)
                return;

            auto& handlers = Get()._handlers[EventType::GetStaticType()];

            auto it = std::remove_if(handlers.begin(),
                                     handlers.end(),
                                     [handlerID](const std::shared_ptr<EventHandlerInterface>& handler)
                                     { return handler->GetID() == handlerID; });

            handlers.erase(it, handlers.end());
        }

        template<typename T, typename... Args>
        static void Emit(Args&&... args)
        {
            static_assert(std::is_base_of<Event, T>::value, "T must inherit from Frost::Event");
            Get()._eventQueue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        static void ProcessEvents();
        static EventManager& Get();

    private:
        using HandlerList = std::vector<std::shared_ptr<EventHandlerInterface>>;

        std::vector<std::unique_ptr<Event>> _eventQueue;
        std::unordered_map<EventType, HandlerList> _handlers;
    };
} // namespace Frost