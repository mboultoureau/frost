#include "Frost/Debugging/DebugInterface/DebugInput.h"
#include "Frost/Input/Input.h"

#include <imgui.h>
#include <string>

namespace Frost
{
    void DebugInput::OnImGuiRender(float deltaTime)
    {
        if (ImGui::CollapsingHeader("Input"))
        {
            _DrawMousePanel();
            ImGui::Separator();

            _DrawKeyboardPanel();
            ImGui::Separator();

            if (ImGui::TreeNode("Gamepads"))
            {
                for (uint8_t i = 0; i < Gamepad::MAX_GAMEPADS; ++i)
                {
                    Gamepad& gamepad = Input::GetGamepad(i);
                    _DrawGamepadPanel(gamepad);
                }
                ImGui::TreePop();
            }
        }
    }

    void DebugInput::_DrawMousePanel()
    {
        bool isCursorVisible = Input::GetMouse().IsCursorVisible();

        if (ImGui::TreeNode("Mouse"))
        {
            if (ImGui::Checkbox("Show Mouse Cursor", &isCursorVisible))
            {
                if (isCursorVisible)
                {
                    Input::GetMouse().ShowCursor();
                }
                else
                {
                    Input::GetMouse().HideCursor();
                }
            }

            ImGui::Separator();

            // Position and Scroll
            Mouse::MousePosition mousePosition = Input::GetMouse().GetPosition();
            Mouse::MouseViewportPosition mousePositionViewport = Input::GetMouse().GetViewportPosition();
            Mouse::MouseScroll mouseScroll = Input::GetMouse().GetScroll();

            if (ImGui::TreeNode("Position and Scroll"))
            {
                ImGui::Text("Screen Position: (%u, %u)", mousePosition.x, mousePosition.y);
                ImGui::Text("Viewport Position: (%.2f, %.2f)", mousePositionViewport.x, mousePositionViewport.y);
                ImGui::Text("Scroll Wheel: X=%d, Y=%d", mouseScroll.scrollX, mouseScroll.scrollY);

                ImGui::Separator();
                _DrawMouseVisual("Screen Position Visual", 150.0f);

                ImGui::TreePop();
            }

            ImGui::Separator();

            // Button states
            if (ImGui::TreeNode("Button States"))
            {
                auto GetButtonStateString = [](Mouse::ButtonState state) -> const char*
                {
                    switch (state)
                    {
                        case Mouse::ButtonState::Released:
                            return "Released";
                        case Mouse::ButtonState::Hold:
                            return "Hold";
                        case Mouse::ButtonState::Pressed:
                            return "Pressed (Down)";
                        case Mouse::ButtonState::Up:
                            return "Up (Just Released)";
                        default:
                            return "Unknown";
                    }
                };

                auto GetButtonName = [](Mouse::MouseBoutton button) -> const char*
                {
                    switch (button)
                    {
                        case Mouse::MouseBoutton::Left:
                            return "Left";
                        case Mouse::MouseBoutton::Middle:
                            return "Middle";
                        case Mouse::MouseBoutton::Right:
                            return "Right";
                        case Mouse::MouseBoutton::XButton1:
                            return "XButton1";
                        case Mouse::MouseBoutton::XButton2:
                            return "XButton2";
                        default:
                            return "Unknown";
                    }
                };

                for (uint8_t i = 0; i < (uint8_t)Mouse::MouseBoutton::Count; ++i)
                {
                    Mouse::MouseBoutton button = (Mouse::MouseBoutton)i;
                    Mouse::ButtonState state = Input::GetMouse().GetButtonState(button);

                    const char* stateStr = GetButtonStateString(state);
                    const char* buttonName = GetButtonName(button);

                    // Default: White, Pressed: Green, Hold: Yellow
                    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    if (state == Mouse::ButtonState::Pressed)
                    {
                        color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                    }
                    else if (state == Mouse::ButtonState::Hold)
                    {
                        color = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
                    }

                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("%s: %s", buttonName, stateStr);
                    ImGui::PopStyleColor();
                }
                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
    }

    void DebugInput::_DrawKeyboardPanel()
    {
        if (ImGui::TreeNode("Keyboard"))
        {
            auto GetKeyStateString = [](KeyState state) -> const char*
            {
                switch (state)
                {
                    case KeyState::DOWN:
                        return "DOWN (Pressed)";
                    case KeyState::REPEATED:
                        return "HOLDING";
                    case KeyState::UP:
                        return "IDLE/UP";
                    default:
                        return "UNKNOWN";
                }
            };

            auto DrawKeyState = [&](VirtualKeyCode keyCode, const char* name)
            {
                KeyState state = Input::GetKeyboard().GetKeyState(keyCode);

                ImVec4 color;
                if (state == KeyState::DOWN)
                    color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                else if (state == KeyState::REPEATED)
                    color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                else // UP
                    color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::Text("%s: %s", name, GetKeyStateString(state));
                ImGui::PopStyleColor();
                ImGui::NextColumn();
            };

            ImGui::Columns(4, "KeyCols", false);

            ImGui::SeparatorText("Mouse Buttons");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            DrawKeyState(VirtualKeyCode::K_LBUTTON, "L-Click");
            DrawKeyState(VirtualKeyCode::K_RBUTTON, "R-Click");
            DrawKeyState(VirtualKeyCode::K_MBUTTON, "M-Click");
            DrawKeyState(VirtualKeyCode::K_XBUTTON1, "XButton1");
            DrawKeyState(VirtualKeyCode::K_XBUTTON2, "XButton2");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            ImGui::SeparatorText("Alphabet (A-Z)");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            for (char c = 'A'; c <= 'Z'; ++c)
            {
                DrawKeyState((VirtualKeyCode)c, std::string(1, c).c_str());
            }

            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::SeparatorText("Numbers (0-9)");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            for (char c = '0'; c <= '9'; ++c)
            {
                DrawKeyState((VirtualKeyCode)c, std::string(1, c).c_str());
            }
            ImGui::NextColumn();
            ImGui::NextColumn();

            ImGui::SeparatorText("Function Keys (F1-F24)");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            for (int i = 1; i <= 24; ++i)
            {
                DrawKeyState((VirtualKeyCode)(VirtualKeyCode::K_F1 + i - 1), ("F" + std::to_string(i)).c_str());
            }

            ImGui::SeparatorText("Numpad");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            DrawKeyState(VirtualKeyCode::K_NUMPAD0, "Numpad 0");
            DrawKeyState(VirtualKeyCode::K_NUMPAD1, "Numpad 1");
            DrawKeyState(VirtualKeyCode::K_NUMPAD2, "Numpad 2");
            DrawKeyState(VirtualKeyCode::K_NUMPAD3, "Numpad 3");
            DrawKeyState(VirtualKeyCode::K_NUMPAD4, "Numpad 4");
            DrawKeyState(VirtualKeyCode::K_NUMPAD5, "Numpad 5");
            DrawKeyState(VirtualKeyCode::K_NUMPAD6, "Numpad 6");
            DrawKeyState(VirtualKeyCode::K_NUMPAD7, "Numpad 7");
            DrawKeyState(VirtualKeyCode::K_NUMPAD8, "Numpad 8");
            DrawKeyState(VirtualKeyCode::K_NUMPAD9, "Numpad 9");
            DrawKeyState(VirtualKeyCode::K_MULTIPLY, "Multiply");
            DrawKeyState(VirtualKeyCode::K_ADD, "Add");
            DrawKeyState(VirtualKeyCode::K_SEPARATOR, "Separator");
            DrawKeyState(VirtualKeyCode::K_SUBTRACT, "Subtract");
            DrawKeyState(VirtualKeyCode::K_DECIMAL, "Decimal");
            DrawKeyState(VirtualKeyCode::K_DIVIDE, "Divide");

            ImGui::SeparatorText("Control & Modifiers");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            DrawKeyState(VirtualKeyCode::K_LSHIFT, "L-Shift");
            DrawKeyState(VirtualKeyCode::K_RSHIFT, "R-Shift");
            DrawKeyState(VirtualKeyCode::K_LCONTROL, "L-Ctrl");
            DrawKeyState(VirtualKeyCode::K_RCONTROL, "R-Ctrl");
            DrawKeyState(VirtualKeyCode::K_LMENU, "L-Alt");
            DrawKeyState(VirtualKeyCode::K_RMENU, "R-Alt");
            DrawKeyState(VirtualKeyCode::K_LWIN, "L-Win");
            DrawKeyState(VirtualKeyCode::K_RWIN, "R-Win");
            DrawKeyState(VirtualKeyCode::K_SPACE, "Space");
            DrawKeyState(VirtualKeyCode::K_TAB, "Tab");
            DrawKeyState(VirtualKeyCode::K_RETURN, "Enter");
            DrawKeyState(VirtualKeyCode::K_ESCAPE, "Esc");
            DrawKeyState(VirtualKeyCode::K_CAPITAL, "Caps Lock");
            DrawKeyState(VirtualKeyCode::K_NUMLOCK, "Num Lock");
            DrawKeyState(VirtualKeyCode::K_SCROLL, "Scroll Lock");
            DrawKeyState(VirtualKeyCode::K_SNAPSHOT, "Print Screen");

            ImGui::SeparatorText("Navigation");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            DrawKeyState(VirtualKeyCode::K_LEFT, "Arrow Left");
            DrawKeyState(VirtualKeyCode::K_UP, "Arrow Up");
            DrawKeyState(VirtualKeyCode::K_RIGHT, "Arrow Right");
            DrawKeyState(VirtualKeyCode::K_DOWN, "Arrow Down");
            DrawKeyState(VirtualKeyCode::K_HOME, "Home");
            DrawKeyState(VirtualKeyCode::K_END, "End");
            DrawKeyState(VirtualKeyCode::K_PRIOR, "Page Up");
            DrawKeyState(VirtualKeyCode::K_NEXT, "Page Down");
            DrawKeyState(VirtualKeyCode::K_INSERT, "Insert");
            DrawKeyState(VirtualKeyCode::K_DELETE, "Delete");
            DrawKeyState(VirtualKeyCode::K_BACK, "Backspace");
            DrawKeyState(VirtualKeyCode::K_PAUSE, "Pause");

            ImGui::SeparatorText("Media & OEM (Partial)");
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();
            ImGui::NextColumn();

            DrawKeyState(VirtualKeyCode::K_VOLUME_MUTE, "Mute");
            DrawKeyState(VirtualKeyCode::K_VOLUME_DOWN, "Volume Down");
            DrawKeyState(VirtualKeyCode::K_VOLUME_UP, "Volume Up");
            DrawKeyState(VirtualKeyCode::K_MEDIA_PLAY_PAUSE, "Play/Pause");
            DrawKeyState(VirtualKeyCode::K_OEM_PLUS, "+ / =");
            DrawKeyState(VirtualKeyCode::K_OEM_MINUS, "- / _");
            DrawKeyState(VirtualKeyCode::K_OEM_COMMA, ", / <");
            DrawKeyState(VirtualKeyCode::K_OEM_PERIOD, ". / >");
            DrawKeyState(VirtualKeyCode::K_OEM_1, "; / :");
            DrawKeyState(VirtualKeyCode::K_OEM_2, "/ / ?");
            DrawKeyState(VirtualKeyCode::K_OEM_3, "` / ~");
            DrawKeyState(VirtualKeyCode::K_OEM_4, "[ / {");
            DrawKeyState(VirtualKeyCode::K_OEM_5, "\\ / |");
            DrawKeyState(VirtualKeyCode::K_OEM_6, "] / }");
            DrawKeyState(VirtualKeyCode::K_OEM_7, "' / \"");

            ImGui::Columns(1);
            ImGui::TreePop();
        }
    }

    void DebugInput::_DrawGamepadPanel(Gamepad& gamepad)
    {
        ImGui::PushID(gamepad.GetId());
        std::string name = "Controller " + std::to_string(gamepad.GetId());
        if (ImGui::CollapsingHeader(name.c_str()))
        {
            if (!gamepad.IsConnected())
            {
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: Disconnected");
            }
            else
            {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Status: Connected");

                ImGui::Separator();
                if (ImGui::TreeNode("Buttons"))
                {
                    // Simple representation of XInput buttons
                    ImGui::Columns(4, "ButtonCols", false);
                    auto DrawButtonState = [&](Gamepad::Buttons button, const char* name)
                    {
                        ImGui::TextColored(gamepad.IsButtonPressed(button) ? ImVec4(1.0f, 0.8f, 0.0f, 1.0f)
                                                                           : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                                           "%s: %s",
                                           name,
                                           gamepad.IsButtonPressed(button) ? "PRESS" : "UP");
                        ImGui::NextColumn();
                    };

                    DrawButtonState(Gamepad::BUTTON_A, "A");
                    DrawButtonState(Gamepad::BUTTON_B, "B");
                    DrawButtonState(Gamepad::BUTTON_X, "X");
                    DrawButtonState(Gamepad::BUTTON_Y, "Y");

                    DrawButtonState(Gamepad::BUTTON_DPAD_UP, "D-Up");
                    DrawButtonState(Gamepad::BUTTON_DPAD_DOWN, "D-Down");
                    DrawButtonState(Gamepad::BUTTON_DPAD_LEFT, "D-Left");
                    DrawButtonState(Gamepad::BUTTON_DPAD_RIGHT, "D-Right");

                    DrawButtonState(Gamepad::BUTTON_START, "Start");
                    DrawButtonState(Gamepad::BUTTON_BACK, "Back");
                    DrawButtonState(Gamepad::BUTTON_LEFT_SHOULDER, "LB");
                    DrawButtonState(Gamepad::BUTTON_RIGHT_SHOULDER, "RB");

                    DrawButtonState(Gamepad::BUTTON_LEFT_THUMB, "L-Stick");
                    DrawButtonState(Gamepad::BUTTON_RIGHT_THUMB, "R-Stick");

                    ImGui::Columns(1);
                    ImGui::TreePop();
                }

                ImGui::Separator();
                if (ImGui::TreeNode("Joysticks & Triggers"))
                {
                    Gamepad::Joystick leftJoy = gamepad.GetLeftJoystick();
                    Gamepad::Joystick rightJoy = gamepad.GetRightJoystick();
                    Gamepad::Trigger leftTrig = gamepad.GetLeftTrigger();
                    Gamepad::Trigger rightTrig = gamepad.GetRightTrigger();

                    // Joysticks
                    ImGui::BeginGroup();
                    _DrawJoystickVisual("Left Stick", leftJoy, 40.0f);
                    ImGui::EndGroup();
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 4.0f);

                    ImGui::BeginGroup();
                    _DrawJoystickVisual("Right Stick", rightJoy, 40.0f);
                    ImGui::EndGroup();

                    ImGui::Text("Left Stick (Norm): (%.2f, %.2f) | Mag: %.2f",
                                leftJoy.normalizedX,
                                leftJoy.normalizedY,
                                leftJoy.normalizedMagnitude);
                    ImGui::Text("Right Stick (Norm): (%.2f, %.2f) | Mag: %.2f",
                                rightJoy.normalizedX,
                                rightJoy.normalizedY,
                                rightJoy.normalizedMagnitude);

                    ImGui::SeparatorText("Joystick Deadzones (Max Raw: 32767)");

                    // Joysticks deadzones
                    int currentLStickDZ = leftJoy.deadZone;
                    if (ImGui::SliderInt("L Stick Deadzone (Raw)", &currentLStickDZ, 0, 32767))
                    {
                        gamepad.SetDeadZoneLeftJoystick(currentLStickDZ);
                    }

                    int currentRStickDZ = rightJoy.deadZone;
                    if (ImGui::SliderInt("R Stick Deadzone (Raw)", &currentRStickDZ, 0, 32767))
                    {
                        gamepad.SetDeadZoneRightJoystick(currentRStickDZ);
                    }

                    ImGui::Text("Current Deadzone: L:%d (%.2f%%) | R:%d (%.2f%%)",
                                leftJoy.deadZone,
                                (float)leftJoy.deadZone / 32767.0f * 100.0f,
                                rightJoy.deadZone,
                                (float)rightJoy.deadZone / 32767.0f * 100.0f);

                    ImGui::SameLine();
                    if (ImGui::Button("Reset Sticks DZ"))
                    {
                        gamepad.SetDeadZoneLeftJoystick(Gamepad::DEADZONE_DEFAULT_LEFT_STICK);
                        gamepad.SetDeadZoneRightJoystick(Gamepad::DEADZONE_DEFAULT_RIGHT_STICK);
                    }

                    // Joysticks transforms
                    ImGui::SeparatorText("Joystick Transforms");

                    Gamepad::Transform currentLStickTransform = gamepad.GetTransformLeftJoystick();
                    if (_DrawTransformControl("L Stick Transform", currentLStickTransform))
                    {
                        gamepad.SetTransformLeftJoystick(currentLStickTransform);
                    }

                    Gamepad::Transform currentRStickTransform = gamepad.GetTransformRightJoystick();
                    if (_DrawTransformControl("R Stick Transform", currentRStickTransform))
                    {
                        gamepad.SetTransformRightJoystick(currentRStickTransform);
                    }

                    ImGui::Separator();

                    // Triggers
                    char overlayL[32];
                    snprintf(overlayL, sizeof(overlayL), "%.2f", leftTrig.normalizedValue);
                    ImGui::Text("Left Trigger (LT)");
                    ImVec2 p_lt = ImGui::GetCursorScreenPos();
                    ImGui::ProgressBar(leftTrig.normalizedValue, ImVec2(-1.0f, 20.0f), overlayL);

                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    float deadZoneNormL = (float)leftTrig.deadZone / 255.0f;
                    float barWidth = ImGui::GetContentRegionAvail().x + ImGui::GetStyle().FramePadding.x * 2.0f;
                    float deadZoneWidthL = barWidth * deadZoneNormL;

                    draw_list->AddRectFilled(ImVec2(p_lt.x, p_lt.y),
                                             ImVec2(p_lt.x + deadZoneWidthL, p_lt.y + 20.0f),
                                             IM_COL32(255, 50, 50, 100));

                    char overlayR[32];
                    snprintf(overlayR, sizeof(overlayR), "%.2f", rightTrig.normalizedValue);
                    ImGui::Text("Right Trigger (RT)");
                    ImVec2 p_rt = ImGui::GetCursorScreenPos();
                    ImGui::ProgressBar(rightTrig.normalizedValue, ImVec2(-1.0f, 20.0f), overlayR);

                    float deadZoneNormR = (float)rightTrig.deadZone / 255.0f;
                    float deadZoneWidthR = barWidth * deadZoneNormR;

                    draw_list->AddRectFilled(ImVec2(p_rt.x, p_rt.y),
                                             ImVec2(p_rt.x + deadZoneWidthR, p_rt.y + 20.0f),
                                             IM_COL32(255, 50, 50, 100));

                    ImGui::SeparatorText("Trigger Deadzones (Max Raw: 255)");

                    // Triggers deadzones
                    int currentLTriggerDZ = leftTrig.deadZone;
                    if (ImGui::SliderInt("L Trigger Deadzone (Raw)", &currentLTriggerDZ, 0, 255))
                    {
                        gamepad.SetDeadZoneLeftTrigger(currentLTriggerDZ);
                    }

                    int currentRTriggerDZ = rightTrig.deadZone;
                    if (ImGui::SliderInt("R Trigger Deadzone (Raw)", &currentRTriggerDZ, 0, 255))
                    {
                        gamepad.SetDeadZoneRightTrigger(currentRTriggerDZ);
                    }

                    ImGui::Text("Current Deadzone: L:%d (%.2f%%) | R:%d (%.2f%%)",
                                leftTrig.deadZone,
                                (float)leftTrig.deadZone / 255.0f * 100.0f,
                                rightTrig.deadZone,
                                (float)rightTrig.deadZone / 255.0f * 100.0f);

                    ImGui::SameLine();
                    if (ImGui::Button("Reset Triggers DZ"))
                    {
                        gamepad.SetDeadZoneLeftTrigger(Gamepad::DEADZONE_DEFAULT_TRIGGER);
                        gamepad.SetDeadZoneRightTrigger(Gamepad::DEADZONE_DEFAULT_TRIGGER);
                    }

                    ImGui::TreePop();
                }

                ImGui::Separator();
                if (ImGui::TreeNode("Vibration (Motors)"))
                {
                    // Use a normalized float (0.0f to 1.0f) for easier UI
                    // control
                    float leftNorm = gamepad.GetLeftMotorSpeed() / 65535.0f;
                    float rightNorm = gamepad.GetRightMotorSpeed() / 65535.0f;

                    ImGui::Text("Motor Speeds (0-65535): L:%hu | R:%hu",
                                gamepad.GetLeftMotorSpeed(),
                                gamepad.GetRightMotorSpeed());

                    // Sliders for control
                    if (ImGui::SliderFloat("Left Motor (Heavy)", &leftNorm, 0.0f, 1.0f, "%.2f"))
                    {
                        gamepad.VibrateNormalized(leftNorm, rightNorm);
                    }

                    if (ImGui::SliderFloat("Right Motor (Light)", &rightNorm, 0.0f, 1.0f, "%.2f"))
                    {
                        gamepad.VibrateNormalized(leftNorm, rightNorm);
                    }

                    if (ImGui::Button("Stop Vibration"))
                    {
                        gamepad.StopVibration();
                    }

                    ImGui::TreePop();
                }
            }
        }
        ImGui::PopID();
    }

    /**
     * @see https://github.com/ocornut/imgui/issues/5186
     */
    void DebugInput::_DrawJoystickVisual(const char* label, const Gamepad::Joystick& joy, float radius)
    {
        ImGui::Text("%s", label);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGuiStyle& style = ImGui::GetStyle();

        float draw_size = radius * 2.0f + style.FramePadding.x * 2.0f;
        ImGui::Dummy(ImVec2(draw_size, draw_size));

        ImVec2 center(p.x + radius + style.FramePadding.x, p.y + radius + style.FramePadding.y);

        draw_list->AddCircleFilled(center, radius, IM_COL32(50, 50, 50, 150), 24);

        constexpr float MAX_RAW_STICK_VALUE = 32767.0f;
        float deadZoneRadius = radius * ((float)joy.deadZone / MAX_RAW_STICK_VALUE);

        draw_list->AddCircleFilled(center, deadZoneRadius, IM_COL32(255, 100, 0, 100), 12);
        draw_list->AddCircle(center, deadZoneRadius, IM_COL32(255, 100, 0, 200), 12, 1.0f);

        float crosshair_len = 5.0f;
        draw_list->AddLine(ImVec2(center.x - crosshair_len, center.y),
                           ImVec2(center.x + crosshair_len, center.y),
                           IM_COL32(150, 150, 150, 255));
        draw_list->AddLine(ImVec2(center.x, center.y - crosshair_len),
                           ImVec2(center.x, center.y + crosshair_len),
                           IM_COL32(150, 150, 150, 255));

        float stick_x = center.x + joy.normalizedX * radius;
        float stick_y = center.y - joy.normalizedY * radius;
        float stick_radius = 8.0f;

        ImU32 stick_color = IM_COL32(
            (int)(100 + 155 * joy.normalizedMagnitude), (int)(200 - 100 * joy.normalizedMagnitude), (int)(50), 255);

        draw_list->AddCircleFilled(ImVec2(stick_x, stick_y), stick_radius, stick_color, 12);
    }

    bool DebugInput::_DrawTransformControl(const char* label, Frost::Gamepad::Transform& currentTransform)
    {
        const char* transformTypes[] = {
            "Linear (f(x)=x)", "Quadratic (f(x)=x^2)", "Cubic (f(x)=x^3)", "Square Root (f(x)=sqrt(x))"
        };
        int currentType = (int)currentTransform;
        if (ImGui::Combo(label, &currentType, transformTypes, IM_ARRAYSIZE(transformTypes)))
        {
            currentTransform = (Frost::Gamepad::Transform)currentType;
            return true;
        }
        return false;
    }

    void DebugInput::_DrawMouseVisual(const char* label, float size)
    {
        ImGui::Text("%s", label);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        ImGuiStyle& style = ImGui::GetStyle();

        float draw_size = size + style.FramePadding.x * 2.0f;
        ImGui::Dummy(ImVec2(draw_size, draw_size));

        ImVec2 rect_min(p.x + style.FramePadding.x, p.y + style.FramePadding.y);
        ImVec2 rect_max(rect_min.x + size, rect_min.y + size);

        draw_list->AddRectFilled(rect_min, rect_max, IM_COL32(50, 50, 50, 150));
        draw_list->AddRect(rect_min, rect_max, IM_COL32(150, 150, 150, 255));

        ImGuiIO& io = ImGui::GetIO();
        float screenWidth = io.DisplaySize.x;
        float screenHeight = io.DisplaySize.y;

        Mouse::MousePosition mousePos = Input::GetMouse().GetPosition();

        if (screenWidth > 0 && screenHeight > 0)
        {
            float normX = (float)mousePos.x / screenWidth;
            float normY = (float)mousePos.y / screenHeight;

            normX = (normX < 0.0f) ? 0.0f : (normX > 1.0f) ? 1.0f : normX;
            normY = (normY < 0.0f) ? 0.0f : (normY > 1.0f) ? 1.0f : normY;

            float vis_x = rect_min.x + normX * size;
            float vis_y = rect_min.y + normY * size;

            draw_list->AddCircleFilled(ImVec2(vis_x, vis_y), 4.0f, IM_COL32(255, 80, 80, 255));
        }
    }
} // namespace Frost