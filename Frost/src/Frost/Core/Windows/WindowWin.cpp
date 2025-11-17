    #include "Frost/Core/Windows/WindowWin.h"
    #include "Frost/Debugging/Assert.h"
    #include "Frost/Input/Input.h"
    #include "Frost/Event/EventManager.h"
    #include "Frost/Event/Events/Window/WindowCloseEvent.h"
    #include "Frost/Event/Events/Window/WindowResizeEvent.h"
    #include "Frost/Core/Application.h"

    #include "imgui.h"
    #include "imgui_impl_win32.h"
    #include "imgui_impl_dx11.h"

    #include <iostream>

    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    namespace Frost
    {
        void _CreateConsoleWindow()
        {
            FT_ENGINE_ASSERT(AllocConsole(), "Can't allocate new console");
            FILE* consoleStdout = nullptr;
            FILE* consoleStderr = nullptr;

            FT_ASSERT(!freopen_s(&consoleStdout, "CONOUT$", "w", stdout), "Can't associated stdout to console");
            FT_ASSERT(!freopen_s(&consoleStderr, "CONOUT$", "w", stderr), "Can't associated stderr to console");

            std::cout.clear();
            std::cerr.clear();

            SetConsoleTitle(L"Frost Engine Console");
        }

	    WindowWin::WindowWin(const WindowTitle title) : Window(title)
	    {
    #ifdef FT_DEBUG
            _CreateConsoleWindow();
    #endif

            Logger::Init();

            /**
            * Register windows class
            * @see https://learn.microsoft.com/en-us/windows/win32/winmsg/using-window-classes
            * @see https://devblogs.microsoft.com/oldnewthing/20211220-00/?p=106044
            */
            WNDCLASSEX wcx;

            wcx.cbSize = sizeof(WNDCLASSEX);
            wcx.style = CS_HREDRAW | CS_VREDRAW;
		    wcx.lpfnWndProc = &_WndProc;
            wcx.cbClsExtra = 0;
            wcx.cbWndExtra = 0;
            wcx.hInstance = GetModuleHandle(NULL);
            wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
            wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcx.lpszMenuName = NULL;
            wcx.lpszClassName = L"FrostWClass";
            wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

		    FT_ENGINE_ASSERT(RegisterClassEx(&wcx), "Window Registration Failed!");
        
            // Create window
            DWORD style = WS_OVERLAPPEDWINDOW;
            DWORD exStyle = 0;
            RECT windowRect = { 0, 0, (LONG)_width, (LONG)_height };

            AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

            UINT actualWidth = windowRect.right - windowRect.left;
            UINT actualHeight = windowRect.bottom - windowRect.top;

            _hwnd = CreateWindowEx(
                0, L"FrostWClass", title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                actualWidth, actualHeight, NULL, NULL, GetModuleHandle(NULL), NULL
            );

            FT_ENGINE_ASSERT(_hwnd != NULL, "Window Creation Failed!");

            RECT rect = {};
            GetClientRect(_hwnd, &rect);
            _width = rect.right - rect.left;
            _height = rect.bottom - rect.top;

            if (_width > 0 && _height > 0)
            {
                EventManager::Emit<Frost::WindowResizeEvent>(_width, _height);
            }

            ShowWindow(_hwnd, SW_SHOWNORMAL);
	    }

        void WindowWin::SetFullscreen(const bool isFullscreen)
        {
            if (isFullscreen == _isFullscreen)
            {
                return;
		    }

		    _isFullscreen = isFullscreen;

            if (isFullscreen)
            {
			    FT_ENGINE_INFO("Switching to fullscreen mode");

			    SetWindowLong(_hwnd, GWL_STYLE, WS_POPUP);
			    SetWindowLong(_hwnd, GWL_EXSTYLE, 0);
			    ShowWindow(_hwnd, SW_MAXIMIZE);
                UpdateWindow(_hwnd);
            }
            else
            {
                FT_ENGINE_INFO("Switching to windowed mode");
                SetWindowLong(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
                SetWindowLong(_hwnd, GWL_EXSTYLE, WS_EX_OVERLAPPEDWINDOW);
                ShowWindow(_hwnd, SW_SHOWNORMAL);
			    UpdateWindow(_hwnd);
            }
        }

        void WindowWin::MainLoop()
        {
            MSG msg;

            while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT)
                {
                    return;
                }
            }

            if (_sizeChanged)
            {
                EventManager::Emit<Frost::WindowResizeEvent>(_width, _height);
                _sizeChanged = false;
            }
        }

        LRESULT CALLBACK WindowWin::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
            if (!window)
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }

            // ImGui specific
            if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            {
                return 0;
            }

            switch (message)
            {
            case WM_KEYDOWN:
            {
                KeyState keyState = (lParam & (1 << 30)) ? KeyState::REPEATED : KeyState::DOWN;
                Input::GetKeyboard().SetKeyState(static_cast<VirtualKeyCode>(wParam), keyState);
                break;
            }
            case WM_KEYUP:
            {
                Input::GetKeyboard().SetKeyState(static_cast<VirtualKeyCode>(wParam), KeyState::UP);
                break;
            }
            case WM_CLOSE:
            {
                EventManager::Emit<WindowCloseEvent>();
                break;
            }
            case WM_SIZE:
            {
                if (wParam != SIZE_MINIMIZED) {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);

				    window->_width = width;
				    window->_height = height;

                    window->_sizeChanged = true;
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

    }