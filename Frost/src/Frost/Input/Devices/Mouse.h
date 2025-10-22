#pragma once

#include <cstdint>
#include <array>

namespace Frost
{
	class Mouse
	{

	public:
		struct MousePosition
		{
			uint32_t x;
			uint32_t y;
		};

		struct MouseViewportPosition
		{
			float x;
			float y;
		};

		struct MouseScroll
		{
			int16_t scrollX;
			int16_t scrollY;
		};

		enum class ButtonState : uint8_t
		{
			Released = 0,
			Hold = 1,
			Pressed = 2,
			Up = 3
		};

		enum class MouseBoutton : uint8_t
		{
			Left = 0,
			Middle = 1,
			Right = 2,
			XButton1 = 3,
			XButton2 = 4,
			Count
		};

	public:
		// Cursor
		void ShowCursor();
		void HideCursor();
		bool IsCursorVisible() const;
		MousePosition GetPosition() const;
		MouseViewportPosition GetViewportPosition() const;

		// Scroll wheel
		void _ResetScroll();
		void SetScroll(MouseScroll scroll);
		MouseScroll GetScroll() const;

		// Buttons
		ButtonState GetButtonState(const MouseBoutton button) const;
		bool IsButtonPressed(const MouseBoutton button) const;
		bool IsButtonReleased(const MouseBoutton button) const;
		bool IsButtonHold(const MouseBoutton button) const;

		// Update
		void Update();

	private:
		MousePosition _position{};
		MouseViewportPosition _viewportPosition{};
		MouseScroll _scroll{};

		void _UpdatePosition();
		void _UpdateButtonStates();
		bool _IsButtonPressed(int virtualKeyCode);

		static constexpr uint8_t BUTTON_COUNT = static_cast<uint8_t>(MouseBoutton::Count);

		std::array<ButtonState, BUTTON_COUNT> _buttonStates{};
		std::array<bool, BUTTON_COUNT> _previousButtonPresses{};
		std::array<bool, BUTTON_COUNT> _currentButtonPresses{};
	};
}

