#pragma once

#include "Frost/Input/PlayerInput.h"

#include <GameInput.h>
#include <wrl.h>
#include <vector>

using namespace GameInput::v3;

class WindowsPlayerInput
{
public:
	WindowsPlayerInput();
	~WindowsPlayerInput();
	
	void ProcessInput();

	// Mouse
	void ShowCursor();
	void HideCursor();

	// Keyboard
	void IsKeyPressed(int key) {}

	// Gamepad

private:
	Microsoft::WRL::ComPtr<IGameInput> _gameInput;
};

