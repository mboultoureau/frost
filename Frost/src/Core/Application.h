#pragma once

#include "Utils/NoCopy.h"
#include "Core/Window.h"

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
	
		static Application& Get();

		enum {
			DEFAULT_WIDTH = 640,
			DEFAULT_HEIGHT = 480
		};

	private:
		HINSTANCE _hInstance;
		std::unique_ptr<Window> _window;
		bool _running;
		static Application* _singleton;
		
		friend class Window;
	};

	Application* CreateApplication(ApplicationEntryPoint entryPoint);
}
