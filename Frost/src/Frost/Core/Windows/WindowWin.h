#pragma once

#include "Frost/Core/Window.h"

#include <windows.h>

namespace Frost
{
	class WindowWin : public Window
	{
	public:
		WindowWin(const WindowTitle title);
		virtual void SetFullscreen(const bool isFullscreen) override;

		void MainLoop() override;
		HWND GetWindowHandle() const { return _hwnd; }

	protected:
		HWND _hwnd;
		bool _sizeChanged = false;
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}