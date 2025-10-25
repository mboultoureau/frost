#include "Frost/Input/Devices/Gamepad.h"
#include "Frost.h"

// Fix for Windows macro conflict
#undef min

#include <algorithm>
#include <Frost/Event/Events/Input/GamepadDisconnectedEvent.h>
#include <Frost/Event/Events/Input/GamepadConnectedEvent.h>

namespace Frost
{
	Gamepad::Gamepad(const GamepadId id) : _id{ id }, _isConnected{ false }, _dwPacketNumber{ 0 }, _buttons{ 0 }, _leftMotorSpeed{ 0 }, _rightMotorSpeed{ 0 }
    {
        // XInput supports up to 4 controllers (0-3)
		FT_ENGINE_ASSERT(id < XUSER_MAX_COUNT, "Gamepad ID must be between 0 and 3 (inclusive).");

		// Defaults for dead zones
		_leftJoystick.deadZone = DEADZONE_DEFAULT_LEFT_STICK;
		_rightJoystick.deadZone = DEADZONE_DEFAULT_RIGHT_STICK;
		_leftTrigger.deadZone = DEADZONE_DEFAULT_TRIGGER;
		_rightTrigger.deadZone = DEADZONE_DEFAULT_TRIGGER;
    }

    void Gamepad::Update()
	{
		DWORD dwResult;
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        dwResult = XInputGetState(static_cast<DWORD>(_id), &state);

        if (dwResult == ERROR_SUCCESS)
        {
            if (state.dwPacketNumber == _dwPacketNumber)
            {
                return;
			}

            // Check if the controller was previously disconnected
            if (!_isConnected)
            {
                Application::Get().GetEventManager().Emit<GamepadConnectedEvent>(GamepadId(_id));
			}

            _isConnected = true;
			_dwPacketNumber = state.dwPacketNumber;

            _UpdateJoystick(_leftJoystick, state.Gamepad.sThumbLX, state.Gamepad.sThumbLY);
			_UpdateJoystick(_rightJoystick, state.Gamepad.sThumbRX, state.Gamepad.sThumbRY);
            _UpdateTrigger(_leftTrigger, state.Gamepad.bLeftTrigger);
            _UpdateTrigger(_rightTrigger, state.Gamepad.bRightTrigger);
			_buttons = state.Gamepad.wButtons;
        }
        else
        {
			// Check if the controller was previously connected
            if (_isConnected)
            {
                Application::Get().GetEventManager().Emit<GamepadDisconnectedEvent>(GamepadId(_id));
            }

			_isConnected = false;
			_dwPacketNumber = 0;
        }
	}

	bool Gamepad::IsConnected() const noexcept
	{
		return _isConnected;
	}

    Gamepad::GamepadId Gamepad::GetId() const noexcept
    {
		return _id;
    }

    void Gamepad::SetDeadZoneLeftJoystick(int deadZone) noexcept
    {
		_leftJoystick.deadZone = deadZone;
    }

    void Gamepad::SetDeadZoneRightJoystick(int deadZone) noexcept
    {
		_rightJoystick.deadZone = deadZone;
    }

    void Gamepad::SetDeadZoneLeftTrigger(int deadZone) noexcept
    {
		_leftTrigger.deadZone = deadZone;
    }

    void Gamepad::SetDeadZoneRightTrigger(int deadZone) noexcept
    {
		_rightTrigger.deadZone = deadZone;
    }

    void Gamepad::SetDeadZoneLeftJoystickNormalized(float deadZone) noexcept
    {
		_leftJoystick.deadZone = static_cast<int>(deadZone * 32767.0f);
    }

    void Gamepad::SetDeadZoneRightJoystickNormalized(float deadZone) noexcept
	{
        _rightJoystick.deadZone = static_cast<int>(deadZone * 32767.0f);
	}

    void Gamepad::SetDeadZoneLeftTriggerNormalized(float deadZone) noexcept
	{
		_leftTrigger.deadZone = static_cast<int>(deadZone * 255.0f);
	}

    void Gamepad::SetDeadZoneRightTriggerNormalized(float deadZone) noexcept
    {
        _rightTrigger.deadZone = static_cast<int>(deadZone * 255.0f);
    }

    void Gamepad::SetTransformLeftTrigger(Transform transform) noexcept
    {
		_leftTrigger.transform = transform;
    }

    void Gamepad::SetTransformRightTrigger(Transform transform) noexcept
    {
		_rightTrigger.transform = transform;
    }

    Gamepad::Transform Gamepad::GetTransformLeftTrigger() const noexcept
    {
		return _leftTrigger.transform;
    }

    Gamepad::Transform Gamepad::GetTransformRightTrigger() const noexcept
    {
		return _rightTrigger.transform;
    }

    void Gamepad::SetTransformLeftJoystick(Transform transform) noexcept
    {
		_leftJoystick.transform = transform;
    }

    void Gamepad::SetTransformRightJoystick(Transform transform) noexcept
	{
		_rightJoystick.transform = transform;
	}

    Gamepad::Transform Gamepad::GetTransformLeftJoystick() const noexcept
	{
		return _leftJoystick.transform;
	}

    Gamepad::Transform Gamepad::GetTransformRightJoystick() const noexcept
	{
		return _rightJoystick.transform;
	}

    bool Gamepad::IsButtonPressed(Buttons button) noexcept
    {
		return (_buttons & button) != 0;
    }

    Gamepad::Joystick Gamepad::GetLeftJoystick() const noexcept
    {
		return _leftJoystick;
    }

    Gamepad::Joystick Gamepad::GetRightJoystick() const noexcept
    {
        return _rightJoystick;
    }

    Gamepad::Trigger Gamepad::GetLeftTrigger() const noexcept
    {
        return _leftTrigger;
    }

    Gamepad::Trigger Gamepad::GetRightTrigger() const noexcept
    {
        return _rightTrigger;
	}

    WORD Gamepad::GetLeftMotorSpeed() const noexcept
    {
        return _leftMotorSpeed;
    }

    WORD Gamepad::GetRightMotorSpeed() const noexcept
    {
        return _rightMotorSpeed;
	}

    void Gamepad::Vibrate(WORD leftMotorSpeed, WORD rightMotorSpeed) noexcept
    {
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = leftMotorSpeed;
		vibration.wRightMotorSpeed = rightMotorSpeed;
		XInputSetState(static_cast<DWORD>(_id), &vibration);

        _leftMotorSpeed = leftMotorSpeed;
        _rightMotorSpeed = rightMotorSpeed;
    }

    void Gamepad::VibrateNormalized(float leftMotorSpeed, float rightMotorSpeed) noexcept
    {
        WORD leftSpeed = static_cast<WORD>(std::clamp(leftMotorSpeed, 0.0f, 1.0f) * 65535.0f);
        WORD rightSpeed = static_cast<WORD>(std::clamp(rightMotorSpeed, 0.0f, 1.0f) * 65535.0f);
		Vibrate(leftSpeed, rightSpeed);
    }

    void Gamepad::StopVibration() noexcept
    {
		Vibrate(0, 0);
    }

    void Gamepad::_UpdateJoystick(Joystick& joystick, SHORT rawX, SHORT rawY)
    {
        const float MAX_RANGE = 32767.0f;

		joystick.x = static_cast<float>(rawX);
		joystick.y = static_cast<float>(rawY);

		joystick.magnitude = sqrtf(joystick.x * joystick.x + joystick.y * joystick.y);
        joystick.normalizedMagnitude = 0.0f;
        joystick.normalizedX = 0.0f;
        joystick.normalizedY = 0.0f;

        if (joystick.magnitude <= joystick.deadZone)
        {
            return;
        }

        if (joystick.magnitude > 0.0f)
        {
            float inverseMagnitude = 1.0f / joystick.magnitude;
            joystick.normalizedX = joystick.x * inverseMagnitude;
            joystick.normalizedY = joystick.y * inverseMagnitude;
        }

        float finalMagnitude = std::min(joystick.magnitude, MAX_RANGE);
        finalMagnitude -= joystick.deadZone;

        float magnitudeRange = MAX_RANGE - joystick.deadZone;
        joystick.normalizedMagnitude = finalMagnitude / magnitudeRange;

        // Transform
        float transformedMagnitude = joystick.normalizedMagnitude;
        switch (joystick.transform)
        {
            case TRANSFORM_LINEAR:
            {
                break;
            }
            case TRANSFORM_CUBIC:
            {
                transformedMagnitude = transformedMagnitude * transformedMagnitude * transformedMagnitude;
                break;
            }
            case TRANSFORM_QUADRATIC:
            {
                transformedMagnitude = transformedMagnitude * transformedMagnitude;
                break;
            }
            case TRANSFORM_SQRT:
            {
                transformedMagnitude = sqrtf(transformedMagnitude);
                break;
            }
        }
        joystick.normalizedMagnitude = transformedMagnitude;

        joystick.normalizedX *= joystick.normalizedMagnitude;
        joystick.normalizedY *= joystick.normalizedMagnitude;
    }

    void Gamepad::_UpdateTrigger(Trigger& trigger, BYTE rawValue)
    {
        trigger.value = rawValue;

        float linearNormalizedValue = 0.0f;
        if (rawValue > trigger.deadZone)
        {
            linearNormalizedValue = static_cast<float>(rawValue - trigger.deadZone) / static_cast<float>(255 - trigger.deadZone);
        }

        float transformedValue = linearNormalizedValue;

        switch (trigger.transform)
        {
            case TRANSFORM_LINEAR:
            {
                break;
            }
            case TRANSFORM_CUBIC:
            {
                transformedValue = transformedValue * transformedValue * transformedValue;
                break;
            }
            case TRANSFORM_QUADRATIC:
            {
                transformedValue = transformedValue * transformedValue;
                break;
            }
            case TRANSFORM_SQRT:
            {
                transformedValue = sqrtf(transformedValue);
                break;
            }
        }

        trigger.normalizedValue = transformedValue;
    }
}