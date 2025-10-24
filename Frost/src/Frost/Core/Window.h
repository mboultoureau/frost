#pragma once

#include "Frost/Renderer/Renderer.h"

#include <string>
#include <memory>
#include <Windows.h>
#include <exception>

namespace Frost
{
	class Renderer;
}

namespace Frost
{
	struct WindowDimensions
	{
		uint32_t width;
		uint32_t height;
	};

	struct WindowSettings
	{
		std::string title;
		WindowDimensions dimensions;

		WindowSettings(const std::string& title, uint32_t width, uint32_t height) :
			title{ title }, dimensions{ width, height }
		{
		}
	};

	class Window
	{
	public:
		Window(const WindowSettings& settings);
		WindowDimensions GetDimensions();

		void Destroy();
		HWND GetHWND() const { return _hwnd; }

	private:
		ATOM _AppRegisterClass();
		void _CreateWindow(const WindowSettings& settings);
		void _CreateConsoleWindow();
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		bool _hasConsole;
		bool _isWindowed;
		HWND _hwnd;
		std::wstring _title;
		static constexpr const wchar_t _CLASS_NAME[] = L"FrostEngine";

		friend class Device;
		friend class Application;
	};
}
