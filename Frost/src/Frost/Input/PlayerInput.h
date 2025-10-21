#pragma once

class PlayerInput
{
public:
	PlayerInput();
	~PlayerInput();

	void ProcessInput();

	// Mouse
	void ShowCursor();
	void HideCursor();

	// Keyboard
	void IsKeyPressed(int key);

	// Gamepad
};