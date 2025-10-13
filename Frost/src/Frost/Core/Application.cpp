#include "Application.h"

#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Event/WindowCloseEvent.h"

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

		WindowSettings settings{ entryPoint.title, Application::DEFAULT_WIDTH, Application::DEFAULT_HEIGHT};
		_window = std::make_unique<Window>(settings);

		UUID closeHandlerID = _eventManager.Subscribe<WindowCloseEvent>(
			[&](WindowCloseEvent& e) -> bool
			{
				_running = false;
				_window->Destroy();
				return true;
			});
	}
	
	Application::~Application()
	{
		_layerStack.Clear();
		std::cout << "Application shutting down..." << std::endl;
	}

	void Application::PushLayer(Layer* layer)
	{
		_layerStack.PushLayer(layer);
	}

	void Application::PopLayer(Layer* layer)
	{
		_layerStack.PopLayer(layer);
	}
	
	void Application::Run()
	{
		while (_running)
		{
			_eventManager.ProcessEvents();

			if (!_running)
			{
				break;
			}

			MSG msg;
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
				{
					_running = false;
					return;
				}

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			for (Layer* layer : _layerStack._layers)
			{
				layer->OnFixedUpdate(0.0f);
				layer->OnUpdate(0.0f);
				layer->OnLateUpdate(0.0f);
			}

			RendererAPI::Present();
		}
	}

	Application& Application::Get()
	{
		assert(_singleton);
		return *_singleton;
	}
}