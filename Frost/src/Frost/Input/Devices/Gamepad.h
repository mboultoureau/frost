#pragma once

#include <windows.h>
#include <XInput.h>

namespace Frost
{
    // For performance reasons, don't call XInputGetState for an 'empty' user
    // slot every frame. We recommend that you space out checks for new
    // controllers every few seconds instead.

    class Gamepad
    {
    public:
        using GamepadId = unsigned char;
        constexpr static GamepadId MAX_GAMEPADS = XUSER_MAX_COUNT;

        enum DeadZone
        {
            DEADZONE_NONE = 0,
            DEADZONE_DEFAULT_LEFT_STICK = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
            DEADZONE_DEFAULT_RIGHT_STICK = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
            DEADZONE_DEFAULT_TRIGGER = XINPUT_GAMEPAD_TRIGGER_THRESHOLD
        };

        enum Buttons
        {
            BUTTON_DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
            BUTTON_DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
            BUTTON_DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
            BUTTON_DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,
            BUTTON_START = XINPUT_GAMEPAD_START,
            BUTTON_BACK = XINPUT_GAMEPAD_BACK,
            BUTTON_LEFT_THUMB = XINPUT_GAMEPAD_LEFT_THUMB,
            BUTTON_RIGHT_THUMB = XINPUT_GAMEPAD_RIGHT_THUMB,
            BUTTON_LEFT_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
            BUTTON_RIGHT_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,
            BUTTON_A = XINPUT_GAMEPAD_A,
            BUTTON_B = XINPUT_GAMEPAD_B,
            BUTTON_X = XINPUT_GAMEPAD_X,
            BUTTON_Y = XINPUT_GAMEPAD_Y
        };

        enum Transform
        {
            TRANSFORM_LINEAR = 0, // Linear scaling (f(x) = x) - Default.
            TRANSFORM_QUADRATIC,  // Quadratic scaling (f(x) = x^2) - Smooth
                                  // start.
            TRANSFORM_CUBIC,      // Cubing scaling (f(x) = x^3) - Racing/driving
                                  // style.
            TRANSFORM_SQRT        // Square root scaling (f(x) = sqrt(x)) - Smooth end.
        };

        struct Joystick
        {
            float x;
            float y;
            float normalizedX;
            float normalizedY;
            float magnitude;
            float normalizedMagnitude;
            int deadZone;
            Transform transform = TRANSFORM_LINEAR;
        };

        struct Trigger
        {
            BYTE value;
            float normalizedValue;
            int deadZone;
            Transform transform = TRANSFORM_LINEAR;
        };

        Gamepad(GamepadId id);

        void Update();

        bool IsConnected() const noexcept;
        GamepadId GetId() const noexcept;

        // Joystick
        Joystick GetLeftJoystick() const noexcept;
        Joystick GetRightJoystick() const noexcept;

        void SetDeadZoneLeftJoystick(int deadZone) noexcept;
        void SetDeadZoneRightJoystick(int deadZone) noexcept;
        void SetDeadZoneLeftJoystickNormalized(float deadZone) noexcept;
        void SetDeadZoneRightJoystickNormalized(float deadZone) noexcept;

        void SetTransformLeftJoystick(Transform transform) noexcept;
        void SetTransformRightJoystick(Transform transform) noexcept;
        Transform GetTransformLeftJoystick() const noexcept;
        Transform GetTransformRightJoystick() const noexcept;

        // Triggers
        Trigger GetLeftTrigger() const noexcept;
        Trigger GetRightTrigger() const noexcept;

        void SetDeadZoneLeftTrigger(int deadZone) noexcept;
        void SetDeadZoneRightTrigger(int deadZone) noexcept;
        void SetDeadZoneLeftTriggerNormalized(float deadZone) noexcept;
        void SetDeadZoneRightTriggerNormalized(float deadZone) noexcept;

        void SetTransformLeftTrigger(Transform transform) noexcept;
        void SetTransformRightTrigger(Transform transform) noexcept;
        Transform GetTransformLeftTrigger() const noexcept;
        Transform GetTransformRightTrigger() const noexcept;

        // Buttons
        bool IsButtonPressed(Buttons button) noexcept;

        // Vibration
        WORD GetLeftMotorSpeed() const noexcept;
        WORD GetRightMotorSpeed() const noexcept;

        void Vibrate(WORD leftMotorSpeed, WORD rightMotorSpeed) noexcept;
        void VibrateNormalized(float leftMotorSpeed, float rightMotorSpeed) noexcept;
        void StopVibration() noexcept;

    private:
        GamepadId _id;
        bool _isConnected;
        DWORD _dwPacketNumber;
        WORD _buttons;

        Joystick _leftJoystick;
        Joystick _rightJoystick;

        Trigger _leftTrigger;
        Trigger _rightTrigger;

        WORD _leftMotorSpeed;
        WORD _rightMotorSpeed;

        void _UpdateJoystick(Joystick& joystick, SHORT rawX, SHORT rawY);
        void _UpdateTrigger(Trigger& trigger, BYTE rawValue);
    };
} // namespace Frost
