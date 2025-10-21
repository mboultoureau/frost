#pragma once

#include "Frost/Event/Event.h"

namespace Frost
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height)
			: _width(width), _height(height) {
		}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "WindowResizeEvent"; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		
		unsigned int GetWidth() const { return _width; }
		unsigned int GetHeight() const { return _height; }

	private:
		unsigned int _width;
		unsigned int _height;
	};
}
