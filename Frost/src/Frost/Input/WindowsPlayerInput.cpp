#include "WindowsPlayerInput.h"

#include <WinUser.h>

using namespace GameInput::v3;

class GameInputCreateFailed{};

WindowsPlayerInput::WindowsPlayerInput()
{
    HRESULT hr = GameInputCreate(_gameInput.GetAddressOf());
    if (FAILED(hr))
    {
        throw GameInputCreateFailed{};
    }
}

WindowsPlayerInput::~WindowsPlayerInput()
{
}

void WindowsPlayerInput::ProcessInput()
{
	Microsoft::WRL::ComPtr<IGameInputReading> currentReading;
	
    if (SUCCEEDED(_gameInput->GetCurrentReading(GameInputKindGamepad, nullptr, &currentReading)))
    {
        // MESSAGEBOXA ON BUTTON A PRESS
		GameInputGamepadState gamepadState;
		currentReading->GetGamepadState(&gamepadState);
		if (gamepadState.buttons & GameInputGamepadA)
		{
			MessageBoxA(nullptr, "Gamepad: 'A' Button Pressed!", "Input Debug", MB_OK | MB_ICONINFORMATION);
		}
    }
}

void WindowsPlayerInput::ShowCursor()
{
    while (::ShowCursor(TRUE) < 0);
}

void WindowsPlayerInput::HideCursor()
{
    while (::ShowCursor(FALSE) >= 0);
}
