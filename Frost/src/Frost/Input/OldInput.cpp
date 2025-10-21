#pragma once

#include <windows.h>      // Defines VK_A, VK_W, etc. and MessageBox
#include <GameInput.h>    // Defines IGameInput, GameInputKeyState, etc.
#include <wrl.h>          // For Microsoft::WRL::ComPtr
#include <corecrt_wstdio.h>

using namespace GameInput::v2;

// Global GameInput interface pointer (using a smart pointer for automatic cleanup)
Microsoft::WRL::ComPtr<IGameInput> g_gameInput;

// Function to initialize GameInput
HRESULT InitializeInput()
{
    // GameInputCreate initializes the system and returns the IGameInput interface.
    return GameInputCreate(g_gameInput.GetAddressOf());
}

// Function to handle keyboard and mouse input
void ProcessInput(HWND hWnd)
{
    if (!g_gameInput)
        return;

    IGameInputReading* reading = nullptr;

    // --- 1. Process Keyboard Input ---
    HRESULT hr = g_gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);

    if (SUCCEEDED(hr) && reading)
    {
        GameInputKeyState keyStates[16];
        uint32_t count = reading->GetKeyCount();

        reading->GetKeyState(16, keyStates);

        for (uint32_t i = 0; i < count; ++i)
        {
            // Use the standard VK_A constant, now defined by <windows.h>
            if (keyStates[i].virtualKey == 'A')
            {
                // MessageBox(hWnd, TEXT("Keyboard: 'A' Key Pressed (using VK_A)!"), TEXT("Input Debug"), MB_OK | MB_ICONINFORMATION);
            }
            // Example for 'W' key
            if (keyStates[i].virtualKey == 'W')
            {
                //MessageBox(hWnd, TEXT("Keyboard: 'W' Key Pressed (using VK_W)!"), TEXT("Input Debug"), MB_OK | MB_ICONINFORMATION);
            }
        }
        reading->Release();
        reading = nullptr;
    }

    // ---------------------------------------------------------------------------------------

        // --- 2. Process Mouse Input ---
    hr = g_gameInput->GetCurrentReading(GameInputKindMouse, nullptr, &reading);

    if (SUCCEEDED(hr) && reading)
    {
        GameInputMouseState mouseState;
        reading->GetMouseState(&mouseState);

        // Check for Left Mouse Button press (GameInputMouseLeft is a GameInput enum)
        if (mouseState.buttons & GameInputMouseLeftButton)
        {
            // Format coordinates (delta values) for the debug message
            wchar_t buffer[128];
            swprintf_s(buffer, 128, L"Mouse: Left Clicked\nDelta Position: (%lld, %lld)",
                mouseState.positionX, mouseState.positionY);

            // MessageBox(hWnd, buffer, TEXT("Input Debug"), MB_OK | MB_ICONEXCLAMATION);
        }

        reading->Release();
    }

    // Process Gamepad
	hr = g_gameInput->GetCurrentReading(GameInputKindGamepad, nullptr, &reading);

    if (SUCCEEDED(hr) && reading)
    {
		GameInputGamepadState gamepadState;
		reading->GetGamepadState(&gamepadState);

		// Check for A button press (GameInputGamepadA is a GameInput enum)
        if (gamepadState.buttons & GameInputGamepadA)
        {
            MessageBox(hWnd, TEXT("Gamepad: 'A' Button Pressed!"), TEXT("Input Debug"), MB_OK | MB_ICONINFORMATION);
		}

	}

}

// --- Pointer Visibility Control (Standard Windows API) ---

void HidePointer()
{
    // Decrement the cursor display count until it is negative (hidden).
    while (ShowCursor(FALSE) >= 0);
}

void ShowPointer()
{
    // Increment the cursor display count until it is zero or positive (visible).
    while (ShowCursor(TRUE) < 0);
}

/*
// --- Conceptual Main Loop (Simplified Win32 structure) ---
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    // In a real application, you would create a game window here.
    // Using GetConsoleWindow() as a placeholder window handle for MessageBox.
    HWND hWnd = GetConsoleWindow();

    if (FAILED(InitializeInput()))
    {
        MessageBox(hWnd, TEXT("GameInput API initialization failed!"), TEXT("Error"), MB_ICONERROR);
        return 1;
    }

    // Hide the pointer for an FPS-style game
    HidePointer();

    // Main game loop (simplified)
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            // Core input processing happens here, usually once per frame
            ProcessInput(hWnd);
            Sleep(16); // Simulate frame delay (approx. 60 FPS)
        }
    }

    // Cleanup: ensure the pointer is visible before exiting
    ShowPointer();

    return (int)msg.wParam;b
}*/