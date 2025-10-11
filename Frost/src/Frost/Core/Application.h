#pragma once

#include "Frost/Utils/NoCopy.h"
#include "Frost/Core/Window.h"
#include "Frost/Core/LayerStack.h"
#include "Frost/Renderer/Mesh.h"

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
	
		static Application& Get();

		std::unique_ptr<Window>& GetWindow() { return _window; }
		const std::unique_ptr<Window>& GetWindow() const { return _window; }

		MeshLibrary& GetMeshLibrary() { return _meshLibrary; }

		enum {
			DEFAULT_WIDTH = 640,
			DEFAULT_HEIGHT = 480
		};

	private:
		HINSTANCE _hInstance;
		std::unique_ptr<Window> _window;
		bool _running;
		LayerStack _layerStack;

		MeshLibrary _meshLibrary;

		static Application* _singleton;
		
		friend class Window;
		friend class Device;
	};

	Application* CreateApplication(ApplicationEntryPoint entryPoint);
}
