#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Core/Timer.h"
#include "Frost/Core/Window.h"
#include "Frost/Core/LayerStack.h"
#include "Frost/Renderer/ModelLibrary.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Physics/PhysicsConfig.h"

#include <Windows.h>
#include <memory>
#include <string>

using namespace std::chrono_literals;

namespace Frost
{
	struct ApplicationEntryPoint
	{
		HINSTANCE hInstance;
		HINSTANCE hPrevInstance;
		PWSTR pCmdLine;
		int nCmdShow;

		std::string title;
	};

	class Application : NoCopy
	{
	public:
		Application(const ApplicationEntryPoint& entryPoint);
		virtual ~Application();
		
		void ConfigurePhysics(const PhysicsConfig& config);
		void Setup();
		void Run();
		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		virtual void OnApplicationReady() {};

		Layer* GetLayer(const Layer::LayerName& name);
	
		static Application& Get();

		std::unique_ptr<Window>& GetWindow() { return _window; }
		const std::unique_ptr<Window>& GetWindow() const { return _window; }

		ModelLibrary& GetModelLibrary() { return _meshLibrary; }
		const ModelLibrary& GetModelLibrary() const { return _meshLibrary; }

		EventManager& GetEventManager() { return _eventManager; }
		const EventManager& GetEventManager() const { return _eventManager; }

		LayerStack& GetLayerStack() { return _layerStack; }
		const LayerStack& GetLayerStack() const { return _layerStack; }

		enum {
			DEFAULT_WIDTH = 640,
			DEFAULT_HEIGHT = 480
		};

	private:
		HINSTANCE _hInstance;
		std::unique_ptr<Window> _window;
		bool _running;

		EventManager _eventManager;
		LayerStack _layerStack;
		ModelLibrary _meshLibrary;
		PhysicsConfig _physicsConfig;
		bool _physicsConfigured = false;

		Timer _renderTimer;
		Timer _physicsTimer;

		Timer::Duration _renderRefreshDuration = 16ms;
		Timer::Duration _physicsRefreshDuration = 16ms;

		static Application* _singleton;
		
		friend class Window;
		friend class Device;
	};

	Application* CreateApplication(ApplicationEntryPoint entryPoint);
}
