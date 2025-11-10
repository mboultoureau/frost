#include "Frost/Core/Window.h"

#include "Frost/Core/Application.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Input/Input.h"

#include <imgui_impl_win32.h>
#include <windowsx.h>
#include <iostream>

namespace Frost
{
    class WindowRegistrationFailed {};
    class WindowCreationFailed {};
    class WindowGetDimensionsFailed{};
	class WindowConsoleCreationFailed {};

	Window::Window(const WindowSettings& settings) : _isWindowed{ true }, _hasConsole{ false }
	{
#ifdef FT_DEBUG
		_CreateConsoleWindow();
#endif

        _title = std::wstring(settings.title.begin(), settings.title.end());

        _AppRegisterClass();
        _CreateWindow(settings);

        Logger::Init();
        FT_ENGINE_INFO("FROST ENGINE");
	}

	ATOM Window::_AppRegisterClass()
	{
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = &_WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = Application::Get()._hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = _CLASS_NAME;
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        ATOM result = RegisterClassEx(&wc);

        if (!result) {
            MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
            throw WindowRegistrationFailed{};
        }
        return result;
	}

    void Window::_CreateWindow(const WindowSettings& settings)
    {
        _hwnd = CreateWindowEx(
            0, _CLASS_NAME, _title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            settings.dimensions.width, settings.dimensions.height, NULL, NULL, GetModuleHandle(NULL), this
        );

        if (_hwnd == NULL) {
            MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
            throw WindowCreationFailed{};
        }

        ShowWindow(_hwnd, SW_SHOWNORMAL);
        UpdateWindow(_hwnd);
    }

    void Window::_CreateConsoleWindow()
    {
        if (AllocConsole()) {
            FILE* consoleStdout = nullptr;
            FILE* consoleStderr = nullptr;

            if (freopen_s(&consoleStdout, "CONOUT$", "w", stdout) != 0) {
				throw WindowConsoleCreationFailed{};
            }

            if (freopen_s(&consoleStderr, "CONOUT$", "w", stderr) != 0) {
                throw WindowConsoleCreationFailed{};
            }

            std::cout.clear();
            std::cerr.clear();

			_hasConsole = true;

            SetConsoleTitle(L"Frost Engine Console");
        }
    }

    void Window::Destroy()
    {
        DestroyWindow(_hwnd);

#ifdef _DEBUG
        if (_hasConsole) {
            FreeConsole();
        }
#endif
	}

    LRESULT CALLBACK Window::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;

        // ProcessInput(hWnd);

        switch (message)
        {
        case WM_KEYDOWN:
        {
            if (lParam & (1 << 30))
            {
                Input::GetKeyboard().SetKeyState(static_cast<VirtualKeyCode>(wParam), KeyState::REPEATED);
            }
            else
            {
                Input::GetKeyboard().SetKeyState(static_cast<VirtualKeyCode>(wParam), KeyState::DOWN);
            }
            break;
        }
        case WM_KEYUP:
        {
            Input::GetKeyboard().SetKeyState(static_cast<VirtualKeyCode>(wParam), KeyState::UP);
            break;
        }
        case WM_CLOSE:
        {
            Application::Get().GetEventManager().Emit<Frost::WindowCloseEvent>();
            break;
        }
        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED) {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);

                Application::Get().GetEventManager().Emit<Frost::WindowResizeEvent>(width, height);
            }
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;
        }
        case WM_MOUSEWHEEL:
        {
            int16_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            Input::GetMouse().SetScroll(Mouse::MouseScroll{
                Input::GetMouse().GetScroll().scrollX,
                delta
            });
            break;
        }
        case WM_MOUSEHWHEEL:
        {
            int16_t delta = GET_WHEEL_DELTA_WPARAM(wParam);
            Input::GetMouse().SetScroll(Mouse::MouseScroll{
                delta,
                Input::GetMouse().GetScroll().scrollY
            });
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    WindowDimensions Window::GetWindowDimensions()
    {
        WindowDimensions dimensions;
        RECT rect;
        
        if (!GetWindowRect(_hwnd, &rect)) {
            MessageBox(NULL, L"Window Get Dimensions Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
            throw WindowGetDimensionsFailed{};
        }

        dimensions.width = rect.right - rect.left;
        dimensions.height = rect.bottom - rect.top;

        return dimensions;
    }

    WindowDimensions Window::GetRenderedZoneDimensions()
    {
        WindowDimensions dimensions;
        RECT rect;

        if (!GetClientRect(_hwnd, &rect)) {
            MessageBox(NULL, L"Window Get Rendered Zone Dimensions Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
            throw WindowGetDimensionsFailed{};
        }

        dimensions.width = rect.right - rect.left;
        dimensions.height = rect.bottom - rect.top;

        return dimensions;
    }
}