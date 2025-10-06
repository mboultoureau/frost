#pragma once

#include "Core/Core.h"
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

	class FT_API Application : NoCopy
	{
	public:
		Application(const ApplicationEntryPoint& entryPoint);
		virtual ~Application();
		void Run();
	
		static Application& Get();

	private:
		HINSTANCE _hInstance;
		std::unique_ptr<Window> _window;
		bool _running;
		static Application* _singleton;
		
		friend class Window;
	};

	Application* CreateApplication(ApplicationEntryPoint entryPoint);
}
