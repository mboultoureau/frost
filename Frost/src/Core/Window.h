#pragma once

#include "Core/Core.h"

#include <string>
#include <memory>
#include <Windows.h>

namespace Frost
{
	struct WindowSettings
	{
		std::string title;
		uint32_t width;
		uint32_t height;

		WindowSettings(const std::string& title, uint32_t width, uint32_t height) :
			title{ title }, width{ width }, height{ height }
		{
		}
	};

	class FT_API Window
	{
	public:
		Window(const WindowSettings& settings);

	private:
		ATOM _AppRegisterClass();
		void _CreateWindow(const WindowSettings& settings);
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		HWND _hwnd;
		std::wstring _title;
		static constexpr const wchar_t* _CLASS_NAME = L"FrostEngine";
	};
}
