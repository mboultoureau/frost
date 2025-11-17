#pragma once

#include "Frost/Event/Event.h"
#include "Frost/Core/Window.h"

namespace Frost
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(Window::WindowSizeUnit width, Window::WindowSizeUnit height)
			: _width(width), _height(height) {
		}

		EventType GetEventType() const override { return GetStaticType(); }
		std::string ToString() const override { return "WindowResizeEvent"; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		
		Window::WindowSizeUnit GetWidth() const { return _width; }
		Window::WindowSizeUnit GetHeight() const { return _height; }

	private:
		Window::WindowSizeUnit _width;
		Window::WindowSizeUnit _height;
	};
}
