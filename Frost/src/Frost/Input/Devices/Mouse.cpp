#include "Frost/Input/Devices/Mouse.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Debugging/Assert.h"

#include <Windows.h>
#include <WinUser.h>
#include <cassert>
#include <iostream>

namespace Frost
{
	// Cursor
	void Mouse::ShowCursor()
	{
		while (::ShowCursor(TRUE) < 0);
	}

	void Mouse::HideCursor()
	{
		while (::ShowCursor(FALSE) >= 0);
	}

	bool Mouse::IsCursorVisible() const
	{
		CURSORINFO cursorInfo = { sizeof(CURSORINFO) };
		if (GetCursorInfo(&cursorInfo))
		{
			return (cursorInfo.flags & CURSOR_SHOWING) != 0;
		}

		return false;
	}

	Mouse::MousePosition Mouse::GetPosition() const
	{
		return _position;
	}

	Mouse::MouseViewportPosition Mouse::GetViewportPosition() const
	{
		return _viewportPosition;
	}

	// Scroll wheel
	void Mouse::_ResetScroll()
	{
		_scroll.scrollX = 0;
		_scroll.scrollY = 0;
	}

	void Mouse::SetScroll(Mouse::MouseScroll scroll)
	{
		_scroll = scroll;
	}

	Mouse::MouseScroll Mouse::GetScroll() const
	{
		return _scroll;
	}

	Mouse::ButtonState Mouse::GetButtonState(const MouseBoutton button) const
	{
		uint8_t buttonIndex = static_cast<uint8_t>(button);
		assert(buttonIndex < BUTTON_COUNT && "Mouse button index out of range!");
		return _buttonStates[buttonIndex];
	}

	bool Mouse::IsButtonPressed(const MouseBoutton button) const
	{
		uint8_t buttonIndex = static_cast<uint8_t>(button);
		assert(buttonIndex < BUTTON_COUNT && "Mouse button index out of range!");
		return _buttonStates[buttonIndex] == ButtonState::Pressed;
	}

	bool Mouse::IsButtonReleased(const MouseBoutton button) const
	{
		uint8_t buttonIndex = static_cast<uint8_t>(button);
		assert(buttonIndex < BUTTON_COUNT && "Mouse button index out of range!");
		return _buttonStates[buttonIndex] == ButtonState::Released;
	}

	bool Mouse::IsButtonHold(const MouseBoutton button) const
	{
		uint8_t buttonIndex = static_cast<uint8_t>(button);
		assert(buttonIndex < BUTTON_COUNT && "Mouse button index out of range!");
		return _buttonStates[buttonIndex] == ButtonState::Hold;
	}

	void Mouse::Update()
	{
		_UpdatePosition();
		_UpdateButtonStates();
		_ResetScroll();
	}

	void Mouse::_UpdatePosition()
	{
		// Position relative to the window (between 0 and window size)
		HWND windowHandle = GetForegroundWindow();
		if (windowHandle == NULL) return;
	
		POINT cursorPosition;
		if (GetCursorPos(&cursorPosition) && ScreenToClient(windowHandle, &cursorPosition))
		{
			_position.x = static_cast<uint32_t>(cursorPosition.x);
			_position.y = static_cast<uint32_t>(cursorPosition.y);
		}

		// Position relative to the viewport (between -1 and 1)
		RECT clientRect;
		if (GetClientRect(windowHandle, &clientRect))
		{
			_viewportPosition.x = (static_cast<float>(cursorPosition.x) / static_cast<float>(clientRect.right)) * 2.0f - 1.0f;
			_viewportPosition.y = 1.0f - (static_cast<float>(cursorPosition.y) / static_cast<float>(clientRect.bottom)) * 2.0f;
		}
	}

	void Mouse::_UpdateButtonStates()
	{
		_previousButtonPresses = _currentButtonPresses;

		_currentButtonPresses[static_cast<uint8_t>(MouseBoutton::Left)] = _IsButtonPressed(VK_LBUTTON);
		_currentButtonPresses[static_cast<uint8_t>(MouseBoutton::Middle)] = _IsButtonPressed(VK_MBUTTON);
		_currentButtonPresses[static_cast<uint8_t>(MouseBoutton::Right)] = _IsButtonPressed(VK_RBUTTON);
		_currentButtonPresses[static_cast<uint8_t>(MouseBoutton::XButton1)] = _IsButtonPressed(VK_XBUTTON1);
		_currentButtonPresses[static_cast<uint8_t>(MouseBoutton::XButton2)] = _IsButtonPressed(VK_XBUTTON2);

		for (uint8_t i = 0; i < BUTTON_COUNT; i++)
		{
			bool wasPressed = _previousButtonPresses[i];
			bool isPressed = _currentButtonPresses[i];

			if (!wasPressed && isPressed)
			{
				_buttonStates[i] = ButtonState::Pressed;
			}
			else if (wasPressed && isPressed)
			{
				_buttonStates[i] = ButtonState::Hold;
			}
			else if (wasPressed && !isPressed)
			{
				_buttonStates[i] = ButtonState::Up;
			}
			else
			{
				_buttonStates[i] = ButtonState::Released;
			}
		}
	}

	bool Mouse::_IsButtonPressed(int virtualKeyCode)
	{
		return (GetAsyncKeyState(virtualKeyCode) & 0x8000) != 0;
	}
}

