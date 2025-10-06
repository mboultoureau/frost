#include "Application.h"

#include <cassert>
#include <iostream>

namespace Frost
{
	Application* Application::_singleton = nullptr;

	Application::Application(const ApplicationEntryPoint& entryPoint):
		_hInstance{ entryPoint.hInstance }, _running{ true }
	{
		assert(!_singleton);
		_singleton = this;

		WindowSettings settings{ entryPoint.title, 640, 480};
		_window = std::make_unique<Window>(settings);
	}
	
	Application::~Application() {}
	
	void Application::Run()
	{
		while (_running)
		{
			MSG msg;
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					_running = false;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	Application& Application::Get()
	{
		return *_singleton;
	}
}