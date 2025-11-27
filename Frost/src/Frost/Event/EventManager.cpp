#include "Frost/Event/EventManager.h"

#include <algorithm>

namespace Frost
{
    void EventManager::ProcessEvents()
    {
        for (auto& event : Get()._eventQueue)
        {
            EventType type = event->GetEventType();

            if (Get()._handlers.count(type))
            {
                auto& handlers = Get()._handlers[type];
                for (const auto& handler : handlers)
                {
                    if (handler && handler->OnEvent(*event))
                    {
                        break;
                    }

                    if (event->IsHandled())
                    {
                        break;
                    }
                }
            }
        }

        Get()._eventQueue.clear();
    }

    EventManager& EventManager::Get()
    {
        static EventManager instance;
        return instance;
    }
} // namespace Frost
