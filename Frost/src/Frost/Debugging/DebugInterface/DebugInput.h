#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"
#include "Frost/Input/Devices/Gamepad.h"

namespace Frost
{
	class DebugInput : public DebugPanel
	{
	public:
		DebugInput() = default;
		virtual ~DebugInput() override = default;
		virtual void OnImGuiRender(float deltaTime) override;
		virtual const char* GetName() const override { return "Input"; }

	private:
		void _DrawMousePanel();
		void _DrawKeyboardPanel();
		void _DrawGamepadPanel(Gamepad& gamepad);
		void _DrawJoystickVisual(const char* label, const Gamepad::Joystick& joy, float radius);
		bool _DrawTransformControl(const char* label, Frost::Gamepad::Transform& currentTransform);
		void _DrawMouseVisual(const char* label, float size);
	};
}