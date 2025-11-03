#include "Frost/Input/Devices/Keyboard.h"

namespace Frost
{
	KeyState Keyboard::GetKeyState(const VirtualKeyCode keyCode) const
	{
		auto it = _keyStates.find(keyCode);
		if (it != _keyStates.end())
		{
			return it->second;
		}
		return KeyState::UP;
	}

	void Keyboard::SetKeyState(const VirtualKeyCode keyCode, const KeyState newState)
	{
		_keyStates[keyCode] = newState;
	}

	bool Keyboard::IsKeyDown(const VirtualKeyCode keyCode) const
	{
		auto it = _keyStates.find(keyCode);
		if (it != _keyStates.end())
		{
			return it->second == KeyState::DOWN || it->second == KeyState::REPEATED;
		}
		return KeyState::DOWN;
	}
}