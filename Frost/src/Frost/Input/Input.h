#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Input/Devices/Gamepad.h"
#include "Frost/Input/Devices/Keyboard.h"
#include "Frost/Input/Devices/Mouse.h"
#include "Frost/Utils/NoCopy.h"

#include <array>

namespace Frost
{
    class FROST_API Input : NoCopy
    {
    public:
        static Input& Get();
        static Mouse& GetMouse();
        static Keyboard& GetKeyboard();
        static Gamepad& GetGamepad(const Gamepad::GamepadId id);

        static void Update();
        static void Reset();

    private:
        Input() = default;

    private:
        Mouse _mouse;
        Keyboard _keyboard;
        std::array<Gamepad, Gamepad::MAX_GAMEPADS> _gamepads{ Gamepad(0), Gamepad(1), Gamepad(2), Gamepad(3) };
    };
} // namespace Frost
