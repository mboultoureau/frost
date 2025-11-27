#pragma once

#include "Frost/Core/Window.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"

#include <windows.h>

namespace Frost
{
    class WindowWin : public Window
    {
    public:
        WindowWin(const WindowTitle title);
        ~WindowWin() override;
        virtual void SetFullscreen(const bool isFullscreen) override;

        void MainLoop() override;
        HWND GetWindowHandle() const { return _hwnd; }

    protected:
        HWND _hwnd;
        bool _sizeChanged = false;
        uint32_t _pendingWidth = 0;
        uint32_t _pendingHeight = 0;
        EventHandlerId _resizeEventHandlerId;

        static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
        bool OnWindowResize(WindowResizeEvent& e);
    };
} // namespace Frost