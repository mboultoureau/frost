#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Event/EventHandler.h"

#include <memory>
#include <vector>

namespace Frost
{
	class EventManager
	{
	public:
		template<typename EventType>
		UUID Subscribe(const typename EventHandler<EventType>::EventCallback& callback)
		{
			static_assert(std::is_base_of<Event, EventType>::value, "EventType must inherit from Frost::Event");

			auto handler = std::make_shared<EventHandler<EventType>>(callback);
			UUID handlerID = handler->GetID();

			SubscribeInternal(EventType::GetStaticType(), std::move(handler));

			return handlerID;
		}
		void Unsubscribe(EventType type, UUID handlerID);

		template<typename T, typename... Args>
		void PushEvent(Args&&... args)
		{
			static_assert(std::is_base_of<Event, T>::value, "T must inherit from Frost::Event");
			_eventQueue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
		}

		void ProcessEvents();

	private:
		using HandlerList = std::vector<std::shared_ptr<EventHandlerInterface>>;

		void SubscribeInternal(EventType type, std::shared_ptr<EventHandlerInterface> handler);

		std::vector<std::unique_ptr<Event>> _eventQueue;
		std::unordered_map<EventType, HandlerList> _handlers;
	};
}