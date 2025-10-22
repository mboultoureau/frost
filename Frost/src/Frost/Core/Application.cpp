#include "Application.h"

#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Frost/Input/Input.h"

#include <cassert>
#include <iostream>

namespace Frost
{
	class NoLayerWithThisName {};

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

	Layer* Application::GetLayer(const Layer::LayerName& name)
	{
		for (Layer* layer : _layerStack._layers)
		{
			if (layer->GetName() == name)
			{
				return layer;
			}
		}

		throw NoLayerWithThisName{};
	}
	
	void Application::Run()
	{
		_renderTimer.Start();
		_physicsTimer.Start();

		while (_running)
		{
			Input::Update();
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

			Timer::Duration _physicsDuration = _physicsTimer.GetDuration();
			if (_physicsDuration >= _physicsRefreshDuration)
			{
				_physicsTimer.Start();

				for (Layer* layer : _layerStack._layers)
				{
					float fixedDeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(_physicsDuration).count();

					layer->OnFixedUpdate(fixedDeltaTime);
				}
			}

			Timer::Duration _renderDuration = _renderTimer.GetDuration();
			if (_renderDuration >= _renderRefreshDuration)
			{
				_renderTimer.Start();

				for (Layer* layer : _layerStack._layers)
				{
					float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(_renderDuration).count();

					layer->OnUpdate(deltaTime);
					layer->OnLateUpdate(deltaTime);
				}

				RendererAPI::Present();
			}
		}
	}

	Application& Application::Get()
	{
		assert(_singleton);
		return *_singleton;
	}
}