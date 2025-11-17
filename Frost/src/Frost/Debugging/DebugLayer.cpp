#include "Frost/Debugging/DebugLayer.h"

#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Event/Events/Input/KeyPressedEvent.h"

#include "Frost/Debugging/DebugInterface/DebugWindow.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Debugging/DebugInterface/DebugScene.h"
#include "Frost/Input/Input.h"


#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

namespace Frost
{
	
	DebugLayer::DebugLayer() : Layer(GetStaticName())
	{
	}

	void DebugLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

		// Setup Platform/Renderer backends
#ifdef FT_PLATFORM_WINDOWS
		WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
		RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());

		ImGui_ImplWin32_Init(window->GetWindowHandle());
		ImGui_ImplDX11_Init(renderer->GetDevice(), renderer->GetDeviceContext());
#else
		#error "Platform not supported!"
#endif

		_debugPanels.push_back(std::make_unique<DebugRendering>());
		_debugPanels.push_back(std::make_unique<DebugScene>());
		_debugPanels.push_back(std::make_unique<DebugWindow>());

		/*
		_logTimer.Start();

		EventManager::SubscribeFront<DebugOptionChangedEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnDebugOptionChanged));
		EventManager::SubscribeFront<WindowCloseEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnWindowClose));
		EventManager::SubscribeFront<WindowResizeEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnWindowResize));
		EventManager::SubscribeFront<GamepadConnectedEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnGamepadConnected));
		EventManager::SubscribeFront<GamepadDisconnectedEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnGamepadDisconnected));
			*/
	}

	void DebugLayer::OnDetach()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		_debugPanels.clear();
		//_logTimer.Pause();
	}

	void DebugLayer::OnUpdate(float deltaTime)
	{
		if (Input::GetKeyboard().IsKeyPressed(K_F1))
		{
			_displayDebug = !_displayDebug;
		}


		if (!_displayDebug) return;

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


//
//		float frameTimeMs = deltaTime * 1000.0f;
//
//		// Update history buffer (circular buffer)
//		_frameTimes[_frameTimeHistoryIndex] = frameTimeMs;
//		_frameTimeHistoryIndex = (_frameTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;
//
//		// Update max frame time for graph scaling
//		_maxFrameTime = 0.0f;
//		for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
//		{
//			if (_frameTimes[i] > _maxFrameTime)
//			{
//				_maxFrameTime = _frameTimes[i];
//			}
//		}
//
		if (!ImGui::Begin("Debug", nullptr, 0))
		{
			ImGui::End();
			return;
		}
		
		for (auto& panel : _debugPanels)
		{
			panel->OnImGuiRender(deltaTime);
		}
		//		
		//		_DrawEventLogPanel();
		//		_DrawInputPanel();
		//		_DrawPerformancePanel();
		//		_DrawRenderingOptionsPanel();
		//		_DrawSceneHierarchyPanel();
		//

		ImGui::End();
	}
	
	void DebugLayer::OnLateUpdate(float deltaTime)
	{
		if (!_displayDebug) return;

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void DebugLayer::AddScene(Scene* scene)
	{
		// Find the DebugScenePanel and add the scene
		for (auto& panel : _debugPanels)
		{
			if (auto debugScenePanel = dynamic_cast<DebugScene*>(panel.get()))
			{
				debugScenePanel->AddScene(scene);
				return;
			}
		}
	}

	void DebugLayer::RemoveScene(Scene* scene)
	{
		for (auto& panel : _debugPanels)
		{
			if (auto debugScenePanel = dynamic_cast<DebugScene*>(panel.get()))
			{
				debugScenePanel->RemoveScene(scene);
				return;
			}
		}
	}
//
//	void DebugLayer::OnFixedUpdate(float fixedDeltaTime)
//	{
//		// Record time in milliseconds
//		float timeMs = fixedDeltaTime * 1000.0f;
//
//		// Update history buffer (circular buffer)
//		_fixedUpdateTimes[_fixedUpdateTimeHistoryIndex] = timeMs;
//		_fixedUpdateTimeHistoryIndex = (_fixedUpdateTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;
//
//		// Update max time for graph scaling
//		if (timeMs > _maxFixedUpdateTime)
//		{
//			_maxFixedUpdateTime = timeMs;
//		}
//	}
//
//
//	void DebugLayer::_DrawSceneHierarchyPanel()
//	{

//	}
//
//	void DebugLayer::_DrawEventLogPanel()
//	{
//		if (ImGui::CollapsingHeader("Event Log"))
//		{
//			if (ImGui::Button("Clear Log"))
//			{
//				_eventLog.clear();
//			}
//
//			ImGui::BeginChild("EventScrollRegion");
//
//			for (const auto& entry : _eventLog)
//			{
//				ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
//				if (entry.type == EventType::WindowClose) color = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
//				else if (entry.type == EventType::WindowResize) color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);
//
//				ImGui::PushStyleColor(ImGuiCol_Text, color);
//				ImGui::Text("[%0.3f] [%d] %s", entry.timestamp, (int)entry.type, entry.message.c_str());
//				ImGui::PopStyleColor();
//			}
//
//			ImGui::EndChild();
//		}
//	}
//
//	void DebugLayer::_DrawMousePanel()
//	{
//		bool isCursorVisible = Input::GetMouse().IsCursorVisible();
//
//		if (ImGui::TreeNode("Mouse"))
//		{
//			if (ImGui::Checkbox("Show Mouse Cursor", &isCursorVisible))
//			{
//				if (isCursorVisible)
//				{
//					Input::GetMouse().ShowCursor();
//				}
//				else
//				{
//					Input::GetMouse().HideCursor();
//				}
//			}
//
//			ImGui::Separator();
//
//			// Position and Scroll
//			Mouse::MousePosition mousePosition = Input::GetMouse().GetPosition();
//			Mouse::MouseViewportPosition mousePositionViewport = Input::GetMouse().GetViewportPosition();
//			Mouse::MouseScroll mouseScroll = Input::GetMouse().GetScroll();
//
//			if (ImGui::TreeNode("Position and Scroll"))
//			{
//				ImGui::Text("Screen Position: (%u, %u)", mousePosition.x, mousePosition.y);
//				ImGui::Text("Viewport Position: (%.2f, %.2f)", mousePositionViewport.x, mousePositionViewport.y);
//				ImGui::Text("Scroll Wheel: X=%d, Y=%d", mouseScroll.scrollX, mouseScroll.scrollY);
//
//				ImGui::TreePop();
//			}
//
//			ImGui::Separator();
//
//			// Button states
//			if (ImGui::TreeNode("Button States"))
//			{
//				auto GetButtonStateString = [](Mouse::ButtonState state) -> const char*
//					{
//						switch (state)
//						{
//						case Mouse::ButtonState::Released: return "Released";
//						case Mouse::ButtonState::Hold:     return "Hold";
//						case Mouse::ButtonState::Pressed:  return "Pressed (Down)";
//						case Mouse::ButtonState::Up:       return "Up (Just Released)";
//						default:                           return "Unknown";
//						}
//					};
//
//				auto GetButtonName = [](Mouse::MouseBoutton button) -> const char*
//					{
//						switch (button)
//						{
//						case Mouse::MouseBoutton::Left:    return "Left";
//						case Mouse::MouseBoutton::Middle:  return "Middle";
//						case Mouse::MouseBoutton::Right:   return "Right";
//						case Mouse::MouseBoutton::XButton1: return "XButton1";
//						case Mouse::MouseBoutton::XButton2: return "XButton2";
//						case Mouse::MouseBoutton::Count:   return "Count"; // Should not happen
//						default:                           return "???";
//						}
//					};
//
//				for (uint8_t i = 0; i < (uint8_t)Mouse::MouseBoutton::Count; ++i)
//				{
//					Mouse::MouseBoutton button = (Mouse::MouseBoutton)i;
//					Mouse::ButtonState state = Input::GetMouse().GetButtonState(button);
//
//					const char* stateStr = GetButtonStateString(state);
//					const char* buttonName = GetButtonName(button);
//
//					// Default: White, Pressed: Green, Hold: Yellow
//					ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
//					if (state == Mouse::ButtonState::Pressed)
//					{
//						color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
//					}
//					else if (state == Mouse::ButtonState::Hold)
//					{
//						color = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);
//					}
//
//					ImGui::PushStyleColor(ImGuiCol_Text, color);
//					ImGui::Text("%s: %s", buttonName, stateStr);
//					ImGui::PopStyleColor();
//				}
//				ImGui::TreePop();
//			}
//
//			ImGui::TreePop();
//		}
//	}
//
//	void DebugLayer::_DrawKeyboardPanel()
//	{
//		if (ImGui::TreeNode("Keyboard"))
//		{
//			auto GetKeyStateString = [](Frost::KeyState state) -> const char*
//				{
//					switch (state)
//					{
//					case Frost::KeyState::DOWN:     return "DOWN (Pressed)";
//					case Frost::KeyState::REPEATED: return "HOLDING";
//					case Frost::KeyState::UP:       return "IDLE/UP";
//					default:                        return "UNKNOWN";
//					}
//				};
//
//			auto DrawKeyState = [&](Frost::VirtualKeyCode keyCode, const char* name)
//				{
//					Frost::KeyState state = Input::GetKeyboard().GetKeyState(keyCode);
//
//					ImVec4 color;
//					if (state == Frost::KeyState::DOWN)
//						color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
//					else if (state == Frost::KeyState::REPEATED)
//						color = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
//					else // UP
//						color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
//
//					ImGui::PushStyleColor(ImGuiCol_Text, color);
//					ImGui::Text("%s: %s", name, GetKeyStateString(state));
//					ImGui::PopStyleColor();
//					ImGui::NextColumn();
//				};
//
//			ImGui::Columns(4, "KeyCols", false);
//
//			ImGui::SeparatorText("Mouse Buttons");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			DrawKeyState(Frost::VirtualKeyCode::K_LBUTTON, "L-Click");
//			DrawKeyState(Frost::VirtualKeyCode::K_RBUTTON, "R-Click");
//			DrawKeyState(Frost::VirtualKeyCode::K_MBUTTON, "M-Click");
//			DrawKeyState(Frost::VirtualKeyCode::K_XBUTTON1, "XButton1");
//			DrawKeyState(Frost::VirtualKeyCode::K_XBUTTON2, "XButton2");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			ImGui::SeparatorText("Alphabet (A-Z)");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			for (char c = 'A'; c <= 'Z'; ++c)
//			{
//				DrawKeyState((Frost::VirtualKeyCode)c, std::string(1, c).c_str());
//			}
//
//			ImGui::NextColumn();  ImGui::NextColumn();
//			ImGui::SeparatorText("Numbers (0-9)");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//			for (char c = '0'; c <= '9'; ++c)
//			{
//				DrawKeyState((Frost::VirtualKeyCode)c, std::string(1, c).c_str());
//			}
//			ImGui::NextColumn(); ImGui::NextColumn();
//
//			ImGui::SeparatorText("Function Keys (F1-F24)");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			for (int i = 1; i <= 24; ++i)
//			{
//				DrawKeyState((Frost::VirtualKeyCode)(Frost::VirtualKeyCode::K_F1 + i - 1),
//					("F" + std::to_string(i)).c_str());
//			}
//
//			ImGui::SeparatorText("Numpad");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD0, "Numpad 0");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD1, "Numpad 1");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD2, "Numpad 2");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD3, "Numpad 3");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD4, "Numpad 4");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD5, "Numpad 5");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD6, "Numpad 6");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD7, "Numpad 7");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD8, "Numpad 8");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMPAD9, "Numpad 9");
//			DrawKeyState(Frost::VirtualKeyCode::K_MULTIPLY, "Multiply");
//			DrawKeyState(Frost::VirtualKeyCode::K_ADD, "Add");
//			DrawKeyState(Frost::VirtualKeyCode::K_SEPARATOR, "Separator");
//			DrawKeyState(Frost::VirtualKeyCode::K_SUBTRACT, "Subtract");
//			DrawKeyState(Frost::VirtualKeyCode::K_DECIMAL, "Decimal");
//			DrawKeyState(Frost::VirtualKeyCode::K_DIVIDE, "Divide");
//
//			ImGui::SeparatorText("Control & Modifiers");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			DrawKeyState(Frost::VirtualKeyCode::K_LSHIFT, "L-Shift");
//			DrawKeyState(Frost::VirtualKeyCode::K_RSHIFT, "R-Shift");
//			DrawKeyState(Frost::VirtualKeyCode::K_LCONTROL, "L-Ctrl");
//			DrawKeyState(Frost::VirtualKeyCode::K_RCONTROL, "R-Ctrl");
//			DrawKeyState(Frost::VirtualKeyCode::K_LMENU, "L-Alt");
//			DrawKeyState(Frost::VirtualKeyCode::K_RMENU, "R-Alt");
//			DrawKeyState(Frost::VirtualKeyCode::K_LWIN, "L-Win");
//			DrawKeyState(Frost::VirtualKeyCode::K_RWIN, "R-Win");
//			DrawKeyState(Frost::VirtualKeyCode::K_SPACE, "Space");
//			DrawKeyState(Frost::VirtualKeyCode::K_TAB, "Tab");
//			DrawKeyState(Frost::VirtualKeyCode::K_RETURN, "Enter");
//			DrawKeyState(Frost::VirtualKeyCode::K_ESCAPE, "Esc");
//			DrawKeyState(Frost::VirtualKeyCode::K_CAPITAL, "Caps Lock");
//			DrawKeyState(Frost::VirtualKeyCode::K_NUMLOCK, "Num Lock");
//			DrawKeyState(Frost::VirtualKeyCode::K_SCROLL, "Scroll Lock");
//			DrawKeyState(Frost::VirtualKeyCode::K_SNAPSHOT, "Print Screen");
//
//			ImGui::SeparatorText("Navigation");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			DrawKeyState(Frost::VirtualKeyCode::K_LEFT, "Arrow Left");
//			DrawKeyState(Frost::VirtualKeyCode::K_UP, "Arrow Up");
//			DrawKeyState(Frost::VirtualKeyCode::K_RIGHT, "Arrow Right");
//			DrawKeyState(Frost::VirtualKeyCode::K_DOWN, "Arrow Down");
//			DrawKeyState(Frost::VirtualKeyCode::K_HOME, "Home");
//			DrawKeyState(Frost::VirtualKeyCode::K_END, "End");
//			DrawKeyState(Frost::VirtualKeyCode::K_PRIOR, "Page Up");
//			DrawKeyState(Frost::VirtualKeyCode::K_NEXT, "Page Down");
//			DrawKeyState(Frost::VirtualKeyCode::K_INSERT, "Insert");
//			DrawKeyState(Frost::VirtualKeyCode::K_DELETE, "Delete");
//			DrawKeyState(Frost::VirtualKeyCode::K_BACK, "Backspace");
//			DrawKeyState(Frost::VirtualKeyCode::K_PAUSE, "Pause");
//
//			ImGui::SeparatorText("Media & OEM (Partial)");
//			ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn(); ImGui::NextColumn();
//
//			DrawKeyState(Frost::VirtualKeyCode::K_VOLUME_MUTE, "Mute");
//			DrawKeyState(Frost::VirtualKeyCode::K_VOLUME_DOWN, "Volume Down");
//			DrawKeyState(Frost::VirtualKeyCode::K_VOLUME_UP, "Volume Up");
//			DrawKeyState(Frost::VirtualKeyCode::K_MEDIA_PLAY_PAUSE, "Play/Pause");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_PLUS, "+ / =");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_MINUS, "- / _");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_COMMA, ", / <");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_PERIOD, ". / >");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_1, "; / :");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_2, "/ / ?");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_3, "` / ~");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_4, "[ / {");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_5, "\\ / |");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_6, "] / }");
//			DrawKeyState(Frost::VirtualKeyCode::K_OEM_7, "' / \"");
//
//			ImGui::Columns(1);
//			ImGui::TreePop();
//		}
//	}
//
//	void DebugLayer::_DrawInputPanel()
//	{
//		if(ImGui::CollapsingHeader("Input"))
//		{
//			_DrawMousePanel();
//			ImGui::Separator();
//
//			_DrawKeyboardPanel();
//			ImGui::Separator();
//
//			if (ImGui::TreeNode("Gamepads"))
//			{
//				for (uint8_t i = 0; i < Frost::Gamepad::MAX_GAMEPADS; ++i)
//				{
//					Frost::Gamepad& gamepad = Input::GetGamepad(i);
//					_DrawGamepadPanel(gamepad);
//				}
//				ImGui::TreePop();
//			}
//		}
//	}
//
//	bool DebugLayer::_DrawTransformControl(const char* label, Frost::Gamepad::Transform& currentTransform)
//	{
//		const char* transformTypes[] = { "Linear (f(x)=x)", "Quadratic (f(x)=x^2)", "Cubic (f(x)=x^3)", "Square Root (f(x)=sqrt(x))" };
//		int currentType = (int)currentTransform;
//		if (ImGui::Combo(label, &currentType, transformTypes, IM_ARRAYSIZE(transformTypes)))
//		{
//			currentTransform = (Frost::Gamepad::Transform)currentType;
//			return true;
//		}
//		return false;
//	}
//
//	void DebugLayer::_DrawGamepadPanel(Frost::Gamepad& gamepad)
//	{
//		ImGui::PushID(gamepad.GetId());
//		std::string name = "Controller " + std::to_string(gamepad.GetId());
//		if (ImGui::CollapsingHeader(name.c_str()))
//		{
//			if (!gamepad.IsConnected())
//			{
//				ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Status: Disconnected");
//			}
//			else
//			{
//				ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Status: Connected");
//
//				ImGui::Separator();
//				if (ImGui::TreeNode("Buttons"))
//				{
//					// Simple representation of XInput buttons
//					ImGui::Columns(4, "ButtonCols", false);
//					auto DrawButtonState = [&](Frost::Gamepad::Buttons button, const char* name) {
//						ImGui::TextColored(gamepad.IsButtonPressed(button) ? ImVec4(1.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
//							"%s: %s", name, gamepad.IsButtonPressed(button) ? "PRESS" : "UP");
//						ImGui::NextColumn();
//						};
//
//					DrawButtonState(Frost::Gamepad::BUTTON_A, "A");
//					DrawButtonState(Frost::Gamepad::BUTTON_B, "B");
//					DrawButtonState(Frost::Gamepad::BUTTON_X, "X");
//					DrawButtonState(Frost::Gamepad::BUTTON_Y, "Y");
//
//					DrawButtonState(Frost::Gamepad::BUTTON_DPAD_UP, "D-Up");
//					DrawButtonState(Frost::Gamepad::BUTTON_DPAD_DOWN, "D-Down");
//					DrawButtonState(Frost::Gamepad::BUTTON_DPAD_LEFT, "D-Left");
//					DrawButtonState(Frost::Gamepad::BUTTON_DPAD_RIGHT, "D-Right");
//
//					DrawButtonState(Frost::Gamepad::BUTTON_START, "Start");
//					DrawButtonState(Frost::Gamepad::BUTTON_BACK, "Back");
//					DrawButtonState(Frost::Gamepad::BUTTON_LEFT_SHOULDER, "LB");
//					DrawButtonState(Frost::Gamepad::BUTTON_RIGHT_SHOULDER, "RB");
//
//					DrawButtonState(Frost::Gamepad::BUTTON_LEFT_THUMB, "L-Stick");
//					DrawButtonState(Frost::Gamepad::BUTTON_RIGHT_THUMB, "R-Stick");
//
//					ImGui::Columns(1);
//					ImGui::TreePop();
//				}
//
//				ImGui::Separator();
//				if (ImGui::TreeNode("Joysticks & Triggers"))
//				{
//					Frost::Gamepad::Joystick leftJoy = gamepad.GetLeftJoystick();
//					Frost::Gamepad::Joystick rightJoy = gamepad.GetRightJoystick();
//					Frost::Gamepad::Trigger leftTrig = gamepad.GetLeftTrigger();
//					Frost::Gamepad::Trigger rightTrig = gamepad.GetRightTrigger();
//
//					// Joysticks
//					ImGui::BeginGroup();
//					_DrawJoystickVisual("Left Stick", leftJoy, 40.0f);
//					ImGui::EndGroup();
//					ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 4.0f);
//
//					ImGui::BeginGroup();
//					_DrawJoystickVisual("Right Stick", rightJoy, 40.0f);
//					ImGui::EndGroup();
//
//					ImGui::Text("Left Stick (Norm): (%.2f, %.2f) | Mag: %.2f", leftJoy.normalizedX, leftJoy.normalizedY, leftJoy.normalizedMagnitude);
//					ImGui::Text("Right Stick (Norm): (%.2f, %.2f) | Mag: %.2f", rightJoy.normalizedX, rightJoy.normalizedY, rightJoy.normalizedMagnitude);
//
//					ImGui::SeparatorText("Joystick Deadzones (Max Raw: 32767)");
//
//					// Joysticks deadzones
//					int currentLStickDZ = leftJoy.deadZone;
//					if (ImGui::SliderInt("L Stick Deadzone (Raw)", &currentLStickDZ, 0, 32767))
//					{
//						gamepad.SetDeadZoneLeftJoystick(currentLStickDZ);
//					}
//
//					int currentRStickDZ = rightJoy.deadZone;
//					if (ImGui::SliderInt("R Stick Deadzone (Raw)", &currentRStickDZ, 0, 32767))
//					{
//						gamepad.SetDeadZoneRightJoystick(currentRStickDZ);
//					}
//
//					ImGui::Text("Current Deadzone: L:%d (%.2f%%) | R:%d (%.2f%%)",
//						leftJoy.deadZone, (float)leftJoy.deadZone / 32767.0f * 100.0f,
//						rightJoy.deadZone, (float)rightJoy.deadZone / 32767.0f * 100.0f);
//
//					ImGui::SameLine();
//					if (ImGui::Button("Reset Sticks DZ"))
//					{
//						gamepad.SetDeadZoneLeftJoystick(Gamepad::DEADZONE_DEFAULT_LEFT_STICK);
//						gamepad.SetDeadZoneRightJoystick(Gamepad::DEADZONE_DEFAULT_RIGHT_STICK);
//					}
//
//					// Joysticks transforms
//					ImGui::SeparatorText("Joystick Transforms");
//
//					Gamepad::Transform currentLStickTransform = gamepad.GetTransformLeftJoystick();
//					if (_DrawTransformControl("L Stick Transform", currentLStickTransform))
//					{
//						gamepad.SetTransformLeftJoystick(currentLStickTransform);
//					}
//
//					Gamepad::Transform currentRStickTransform = gamepad.GetTransformRightJoystick();
//					if (_DrawTransformControl("R Stick Transform", currentRStickTransform))
//					{
//						gamepad.SetTransformRightJoystick(currentRStickTransform);
//					}
//
//					ImGui::Separator();
//
//					// Triggers
//					char overlayL[32];
//					snprintf(overlayL, sizeof(overlayL), "%.2f", leftTrig.normalizedValue);
//					ImGui::Text("Left Trigger (LT)");
//					ImVec2 p_lt = ImGui::GetCursorScreenPos();
//					ImGui::ProgressBar(leftTrig.normalizedValue, ImVec2(-1.0f, 20.0f), overlayL);
//
//					ImDrawList* draw_list = ImGui::GetWindowDrawList();
//					float deadZoneNormL = (float)leftTrig.deadZone / 255.0f;
//					float barWidth = ImGui::GetContentRegionAvail().x + ImGui::GetStyle().FramePadding.x * 2.0f; // Total width of the progress bar area
//					float deadZoneWidthL = barWidth * deadZoneNormL;
//
//					draw_list->AddRectFilled(
//						ImVec2(p_lt.x, p_lt.y),
//						ImVec2(p_lt.x + deadZoneWidthL, p_lt.y + 20.0f),
//						IM_COL32(255, 50, 50, 100)
//					);
//
//					char overlayR[32];
//					snprintf(overlayR, sizeof(overlayR), "%.2f", rightTrig.normalizedValue);
//					ImGui::Text("Right Trigger (RT)");
//					ImVec2 p_rt = ImGui::GetCursorScreenPos();
//					ImGui::ProgressBar(rightTrig.normalizedValue, ImVec2(-1.0f, 20.0f), overlayR);
//
//					float deadZoneNormR = (float)rightTrig.deadZone / 255.0f;
//					float deadZoneWidthR = barWidth * deadZoneNormR;
//
//					draw_list->AddRectFilled(
//						ImVec2(p_rt.x, p_rt.y),
//						ImVec2(p_rt.x + deadZoneWidthR, p_rt.y + 20.0f),
//						IM_COL32(255, 50, 50, 100)
//					);
//
//					ImGui::SeparatorText("Trigger Deadzones (Max Raw: 255)");
//
//					// Triggers deadzones
//					int currentLTriggerDZ = leftTrig.deadZone;
//					if (ImGui::SliderInt("L Trigger Deadzone (Raw)", &currentLTriggerDZ, 0, 255))
//					{
//						gamepad.SetDeadZoneLeftTrigger(currentLTriggerDZ);
//					}
//
//					int currentRTriggerDZ = rightTrig.deadZone;
//					if (ImGui::SliderInt("R Trigger Deadzone (Raw)", &currentRTriggerDZ, 0, 255))
//					{
//						gamepad.SetDeadZoneRightTrigger(currentRTriggerDZ);
//					}
//
//					ImGui::Text("Current Deadzone: L:%d (%.2f%%) | R:%d (%.2f%%)",
//						leftTrig.deadZone, (float)leftTrig.deadZone / 255.0f * 100.0f,
//						rightTrig.deadZone, (float)rightTrig.deadZone / 255.0f * 100.0f);
//
//					ImGui::SameLine();
//					if (ImGui::Button("Reset Triggers DZ"))
//					{
//						gamepad.SetDeadZoneLeftTrigger(Gamepad::DEADZONE_DEFAULT_TRIGGER);
//						gamepad.SetDeadZoneRightTrigger(Gamepad::DEADZONE_DEFAULT_TRIGGER);
//					}
//
//					ImGui::TreePop();
//				}
//
//				ImGui::Separator();
//				if (ImGui::TreeNode("Vibration (Motors)"))
//				{
//					// Use a normalized float (0.0f to 1.0f) for easier UI control
//					float leftNorm = gamepad.GetLeftMotorSpeed() / 65535.0f;
//					float rightNorm = gamepad.GetRightMotorSpeed() / 65535.0f;
//
//					ImGui::Text("Motor Speeds (0-65535): L:%hu | R:%hu", gamepad.GetLeftMotorSpeed(), gamepad.GetRightMotorSpeed());
//
//					// Sliders for control
//					if (ImGui::SliderFloat("Left Motor (Heavy)", &leftNorm, 0.0f, 1.0f, "%.2f"))
//					{
//						gamepad.VibrateNormalized(leftNorm, rightNorm);
//					}
//
//					if (ImGui::SliderFloat("Right Motor (Light)", &rightNorm, 0.0f, 1.0f, "%.2f"))
//					{
//						gamepad.VibrateNormalized(leftNorm, rightNorm);
//					}
//
//					if (ImGui::Button("Stop Vibration"))
//					{
//						gamepad.StopVibration();
//					}
//
//					ImGui::TreePop();
//				}
//			}
//		}
//		ImGui::PopID();
//	}
//
//	/**
//	* @see https://github.com/ocornut/imgui/issues/5186
//	*/
//	void DebugLayer::_DrawJoystickVisual(const char* label, const Frost::Gamepad::Joystick& joy, float radius)
//	{
//		ImGui::Text("%s", label);
//
//		ImDrawList* draw_list = ImGui::GetWindowDrawList();
//		ImVec2 p = ImGui::GetCursorScreenPos();
//		ImGuiStyle& style = ImGui::GetStyle();
//
//		float draw_size = radius * 2.0f + style.FramePadding.x * 2.0f;
//		ImGui::Dummy(ImVec2(draw_size, draw_size));
//
//		ImVec2 center(p.x + radius + style.FramePadding.x, p.y + radius + style.FramePadding.y);
//
//		draw_list->AddCircleFilled(center, radius, IM_COL32(50, 50, 50, 150), 24);
//
//		constexpr float MAX_RAW_STICK_VALUE = 32767.0f;
//		float deadZoneRadius = radius * ((float)joy.deadZone / MAX_RAW_STICK_VALUE);
//
//		draw_list->AddCircleFilled(center, deadZoneRadius, IM_COL32(255, 100, 0, 100), 12);
//		draw_list->AddCircle(center, deadZoneRadius, IM_COL32(255, 100, 0, 200), 12, 1.0f);
//
//		float crosshair_len = 5.0f;
//		draw_list->AddLine(ImVec2(center.x - crosshair_len, center.y), ImVec2(center.x + crosshair_len, center.y), IM_COL32(150, 150, 150, 255));
//		draw_list->AddLine(ImVec2(center.x, center.y - crosshair_len), ImVec2(center.x, center.y + crosshair_len), IM_COL32(150, 150, 150, 255));
//
//		float stick_x = center.x + joy.normalizedX * radius;
//		float stick_y = center.y - joy.normalizedY * radius;
//		float stick_radius = 8.0f;
//
//		ImU32 stick_color = IM_COL32(
//			(int)(100 + 155 * joy.normalizedMagnitude),
//			(int)(200 - 100 * joy.normalizedMagnitude),
//			(int)(50),
//			255
//		);
//
//		draw_list->AddCircleFilled(ImVec2(stick_x, stick_y), stick_radius, stick_color, 12);
//	}
//
//	void DebugLayer::_DrawPerformancePanel()
//	{
//		if (ImGui::CollapsingHeader("Performance"))
//		{
//			float currentFrameTime = _frameTimes[(_frameTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];
//			float fps = (currentFrameTime > 0.0f) ? (1000.0f / currentFrameTime) : 0.0f;
//
//			ImGui::Text("Frame Time (Total): %.2f ms (FPS: %.0f)", currentFrameTime, fps);
//
//			float averageFrameTime = 0.0f;
//			for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i) averageFrameTime += _frameTimes[i];
//			averageFrameTime /= FRAME_TIME_HISTORY_SIZE;
//			std::string overlayFrame = "Avg: " + std::to_string(static_cast<int>(averageFrameTime)) + "ms | Max: " + std::to_string(static_cast<int>(_maxFrameTime)) + "ms";
//
//			ImGui::PlotLines("Frame Time (ms)", _frameTimes, FRAME_TIME_HISTORY_SIZE,
//				_frameTimeHistoryIndex, overlayFrame.c_str(),
//				0.0f, _maxFrameTime * 1.2f, ImVec2(0, 80.0f));
//
//			ImGui::Separator();
//
//			float currentFixedTime = _fixedUpdateTimes[(_fixedUpdateTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];
//
//			ImGui::Text("Physics Update: %.2f ms", currentFixedTime);
//
//			float averageFixedUpdateTime = 0.0f;
//			for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
//			{
//				averageFixedUpdateTime += _fixedUpdateTimes[i];
//			}
//			averageFixedUpdateTime /= FRAME_TIME_HISTORY_SIZE;
//
//			std::string overlayFixed = "Avg: " + std::to_string(static_cast<int>(averageFixedUpdateTime)) + "ms | Max: " + std::to_string(static_cast<int>(_maxFixedUpdateTime)) + "ms";
//
//			ImGui::PlotLines(
//				"Physics Time (ms)",
//				_fixedUpdateTimes,
//				FRAME_TIME_HISTORY_SIZE,
//				_fixedUpdateTimeHistoryIndex,
//				overlayFixed.c_str(),
//				0.0f,
//				_maxFixedUpdateTime * 1.2f,
//				ImVec2(0, 80.0f)
//			);
//		}
//	}
//
//	void DebugLayer::_LogEvent(const Event& e, const std::string& message)
//	{
//		using float_seconds = std::chrono::duration<float>;
//		float currentTime = _logTimer.GetDurationAs<float_seconds>().count();
//		
//		if (_eventLog.size() >= MAX_LOG_ENTRIES)
//		{
//			_eventLog.pop_front();
//		}
//
//		_eventLog.emplace_back(LogEntry{ currentTime, message, e.GetEventType() });
//	}
//
//	bool DebugLayer::_OnDebugOptionChanged(DebugOptionChangedEvent& e)
//	{
//		std::stringstream ss;
//		ss << "Option: " << (int)e.GetOptionType() << " changed.";
//		_LogEvent(e, ss.str());
//		return false; // Don't handle the event, just log it
//	}
//
//	bool DebugLayer::_OnWindowClose(WindowCloseEvent& e)
//	{
//		_LogEvent(e, "Window received close request.");
//		return false;
//	}
//
//	bool DebugLayer::_OnWindowResize(WindowResizeEvent& e)
//	{
//		std::stringstream ss;
//		ss << "Window resized to " << e.GetWidth() << "x" << e.GetHeight() << ".";
//		_LogEvent(e, ss.str());
//		return false;
//	}
//
//	bool DebugLayer::_OnGamepadConnected(GamepadConnectedEvent& e)
//	{
//		std::stringstream ss;
//		ss << "Gamepad " << static_cast<int>(e.GetGamepadId()) << " connected.";
//		_LogEvent(e, ss.str());
//		return false;
//	}
//
//	bool DebugLayer::_OnGamepadDisconnected(GamepadDisconnectedEvent& e)
//	{
//		std::stringstream ss;
//		ss << "Gamepad " << static_cast<int>(e.GetGamepadId()) << " disconnected.";
//		_LogEvent(e, ss.str());
//		return false;
//	}
}