#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Core/Window.h"
#include "Frost/Core/LayerStack.h"
#include "Frost/Renderer/MeshLibrary.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Input/WindowsPlayerInput.h"

#include <Windows.h>
#include <memory>
#include <string>

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
		void Run();
		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
		Layer* GetLayer(const Layer::LayerName& name);
	
		static Application& Get();

		std::unique_ptr<Window>& GetWindow() { return _window; }
		const std::unique_ptr<Window>& GetWindow() const { return _window; }

		MeshLibrary& GetMeshLibrary() { return _meshLibrary; }
		const MeshLibrary& GetMeshLibrary() const { return _meshLibrary; }

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
		MeshLibrary _meshLibrary;
		WindowsPlayerInput _playerInput;

		static Application* _singleton;
		
		friend class Window;
		friend class Device;
	};

	Application* CreateApplication(ApplicationEntryPoint entryPoint);
}
