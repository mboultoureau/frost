#include "Frost/Event/EventManager.h"

#include <algorithm>

namespace Frost
{
	void EventManager::Unsubscribe(EventType type, UUID handlerID)
	{
		if (_handlers.count(type) == 0)
			return;

		auto& handlers = _handlers[type];

		auto it = std::remove_if(handlers.begin(), handlers.end(),
			[handlerID](const std::shared_ptr<EventHandlerInterface>& handler) {
				return handler->GetID() == handlerID;
			});

		handlers.erase(it, handlers.end());
	}

	void EventManager::ProcessEvents()
	{
		for (auto& event : _eventQueue)
		{
			EventType type = event->GetEventType();

			if (_handlers.count(type))
			{
				auto& handlers = _handlers[type];
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

		_eventQueue.clear();
	}
}
