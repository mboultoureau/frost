#include "Core/Window.h"
#include "Core/Application.h"

namespace Frost
{

	Window::Window(const WindowSettings& settings)
	{

        _title = std::wstring(settings.title.begin(), settings.title.end());

        _AppRegisterClass();

        _hwnd = CreateWindowEx(
            0, _CLASS_NAME, _title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            settings.width, settings.height, NULL, NULL, GetModuleHandle(NULL), this
        );

        if (_hwnd == NULL) {
            MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
            return;
        }

        ShowWindow(_hwnd, SW_SHOWNORMAL);
        UpdateWindow(_hwnd);
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
            return 1;
        }
        return result;
	}

    LRESULT CALLBACK Window::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_PAINT: {
            // This message is sent when the window needs to be repainted.
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Draw text on the window
            const wchar_t text[] = L"Hello, Windows World!";
            TextOut(hdc, 10, 10, text, static_cast<int>(wcslen(text)));

            EndPaint(hWnd, &ps);
            break;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
}