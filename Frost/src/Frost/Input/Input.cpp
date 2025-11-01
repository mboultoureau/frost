#include "Input.h"

#include "Frost.h"

namespace Frost
{
	Input& Input::Get()
	{
		static Input singleton;
		return singleton;
	}

	Mouse& Input::GetMouse()
	{
		return Get()._mouse;
	}

	Keyboard& Input::GetKeyboard()
	{
		return Get()._keyboard;
	}

	Gamepad& Input::GetGamepad(const Gamepad::GamepadId id)
	{
		FT_ENGINE_ASSERT(id < Gamepad::MAX_GAMEPADS, "Gamepad ID must be between 0 and 3 (inclusive).");
		return Get()._gamepads[id];
	}

	void Input::Update()
	{
		GetMouse().Update();

		// TODO: Update not connected gamepads every few seconds to check for new connections
		for (uint8_t i = 0; i < Gamepad::MAX_GAMEPADS; ++i)
		{
			GetGamepad(i).Update();
		}
	}
}