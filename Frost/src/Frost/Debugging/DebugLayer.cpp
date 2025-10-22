#include "Frost/Debugging/DebugLayer.h"

#include "Frost/Core/Application.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Event/Event.h"
#include "Frost/Scene/Components/GameObjectInfo.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Input/Input.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <sstream>
#include <imgui_internal.h>
#include <chrono>

namespace Frost
{
	DebugLayer::DebugLayer() : Layer(GetStaticName())
	{
	}

	static bool DrawVec3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		bool modified = false;

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		// --- X ---
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) modified = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// --- Y ---
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) modified = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		// --- Z ---
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			values.z = resetValue;
			modified = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) modified = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);
		ImGui::PopID();

		return modified;
	}

	static bool DrawComponentHeader(const char* name)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding);
		ImGui::PopStyleVar();
		return open;
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
		ImGui_ImplWin32_Init(Application::Get().GetWindow()->GetHWND());
		ImGui_ImplDX11_Init(RendererAPI::Get3DDevice(), RendererAPI::GetImmediateContext());

		_logTimer.Start();

		Application::Get().GetEventManager().SubscribeFront<DebugOptionChangedEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnDebugOptionChanged));
		Application::Get().GetEventManager().SubscribeFront<WindowCloseEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnWindowClose));
		Application::Get().GetEventManager().SubscribeFront<WindowResizeEvent>(
			FROST_BIND_EVENT_FN(DebugLayer::_OnWindowResize));
	}

	void DebugLayer::OnDetach()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		_logTimer.Pause();
	}

	void DebugLayer::OnUpdate(float deltaTime)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		float frameTimeMs = deltaTime * 1000.0f;

		// Update history buffer (circular buffer)
		_frameTimes[_frameTimeHistoryIndex] = frameTimeMs;
		_frameTimeHistoryIndex = (_frameTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;

		// Update max frame time for graph scaling
		_maxFrameTime = 0.0f;
		for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
		{
			if (_frameTimes[i] > _maxFrameTime)
			{
				_maxFrameTime = _frameTimes[i];
			}
		}

		bool debug = true;

		ImGui::Begin("Debug", &debug);
		
		_DrawEventLogPanel();
		_DrawInputPanel();
		_DrawPerformancePanel();
		_DrawRenderingOptionsPanel();
		_DrawSceneHierarchyPanel();

		ImGui::End();
	}

	void DebugLayer::OnLateUpdate(float deltaTime)
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void DebugLayer::OnFixedUpdate(float fixedDeltaTime)
	{
		// Record time in milliseconds
		float timeMs = fixedDeltaTime * 1000.0f;

		// Update history buffer (circular buffer)
		_fixedUpdateTimes[_fixedUpdateTimeHistoryIndex] = timeMs;
		_fixedUpdateTimeHistoryIndex = (_fixedUpdateTimeHistoryIndex + 1) % FRAME_TIME_HISTORY_SIZE;

		// Update max time for graph scaling
		if (timeMs > _maxFixedUpdateTime)
		{
			_maxFixedUpdateTime = timeMs;
		}
	}

	void DebugLayer::_DrawRenderingOptionsPanel()
	{
		bool showWireframe = RendererAPI::Get().GetDevice()->IsWireframeEnabled();

		if (ImGui::CollapsingHeader("Rendering"))
		{
			if (ImGui::Checkbox("Show wireframe", &showWireframe))
			{
				Application::Get().GetEventManager().Emit<DebugOptionChangedEvent>(
					Frost::DebugOptionChangedEvent::Options::SHOW_WIREFRAME, showWireframe
				);
				showWireframe = !showWireframe;
			}
		}
	}

	void DebugLayer::_DrawSceneHierarchyPanel()
	{
		if (ImGui::CollapsingHeader("Scenes"))
		{
			if (_scenes.empty())
			{
				ImGui::Text("No scene attached to DebugLayer");
			}

			for (Scene* scene : _scenes)
			{
				if (ImGui::TreeNode(scene->GetName().c_str()))
				{
					for (GameObject::Id gameObjectId : scene->GetECS().GetActiveGameObjects())
					{
						GameObjectInfo* info = scene->GetECS().GetComponent<GameObjectInfo>(gameObjectId);

						static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
						ImGuiTreeNodeFlags node_flags = base_flags;

						bool is_root = (info && info->parentId == GameObject::InvalidId);
						if (!is_root) node_flags |= ImGuiTreeNodeFlags_Leaf;

						const bool is_open = ImGui::TreeNodeEx((void*)(intptr_t)gameObjectId, node_flags, "%s (ID: %llu)", info->name.c_str(), gameObjectId);

						if (is_open)
						{
							if (DrawComponentHeader("GameObject Info"))
							{
								if (info)
								{
									ImGui::Text("ID: %llu", info->id);
									ImGui::Text("Parent ID: %llu", info->parentId);
									char nameBuffer[256];
									strncpy_s(nameBuffer, sizeof(nameBuffer), info->name.c_str(), sizeof(nameBuffer) - 1);
									nameBuffer[sizeof(nameBuffer) - 1] = '\0';
									if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
									{
										info->name = std::string(nameBuffer);
									}
								}
								ImGui::TreePop();
							}

							if (Transform* transform = scene->GetECS().GetComponent<Transform>(gameObjectId))
							{
								if (DrawComponentHeader("Transform (Local)"))
								{
									DrawVec3Control("Position", transform->position);
									DrawVec3Control("Rotation", transform->rotation);
									DrawVec3Control("Scale", transform->scale, 1.0f);
									ImGui::TreePop();
								}
							}

							if (WorldTransform* worldTransform = scene->GetECS().GetComponent<WorldTransform>(gameObjectId))
							{
								if (DrawComponentHeader("World Transform (Read-Only)"))
								{
									ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
									ImGui::Text("Position: (%.2f, %.2f, %.2f)", worldTransform->position.x, worldTransform->position.y, worldTransform->position.z);
									ImGui::Text("Rotation: (%.2f, %.2f, %.2f)", worldTransform->rotation.x, worldTransform->rotation.y, worldTransform->rotation.z);
									ImGui::Text("Scale:    (%.2f, %.2f, %.2f)", worldTransform->scale.x, worldTransform->scale.y, worldTransform->scale.z);
									ImGui::PopStyleColor();
									ImGui::TreePop();
								}
							}

							if (MeshRenderer* meshRenderer = scene->GetECS().GetComponent<MeshRenderer>(gameObjectId))
							{
								if (DrawComponentHeader("Mesh Renderer"))
								{
									ImGui::Text("Filepath: %s", meshRenderer->meshFilepath.c_str());
									ImGui::Text("Mesh Status: %s", (meshRenderer->mesh ? "Loaded" : "Not Loaded (Error or null)"));
									ImGui::TreePop();
								}
							}

							if (Camera* camera = scene->GetECS().GetComponent<Camera>(gameObjectId))
							{
								if (DrawComponentHeader("Camera"))
								{
									const char* projectionTypes[] = { "Perspective", "Orthographic" };
									int currentType = (int)camera->projectionType;
									if (ImGui::Combo("Projection Type", &currentType, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
									{
										camera->projectionType = (Camera::ProjectionType)currentType;
									}

									if (camera->projectionType == Camera::ProjectionType::Perspective)
									{
										float currentFOVDegrees = camera->perspectiveFOV.value();
										ImGui::DragFloat("FOV", &currentFOVDegrees, 0.5f, 30.0f, 120.0f);
										camera->perspectiveFOV = currentFOVDegrees;
									}
									else
									{
										ImGui::DragFloat("Size", &camera->orthographicSize, 0.1f, 1.0f, 100.0f);
									}

									ImGui::Separator();
									ImGui::DragFloat("Near Clip", &camera->nearClip, 0.01f, 0.001f, camera->farClip - 0.1f, "%.3f");
									ImGui::DragFloat("Far Clip", &camera->farClip, 1.0f, camera->nearClip + 0.1f, 10000.0f);

									ImGui::Separator();
									ImGui::Checkbox("Clear On Render", &camera->clearOnRender);
									ImGui::ColorEdit4("Background Color", camera->backgroundColor, ImGuiColorEditFlags_NoAlpha);

									ImGui::Text("Viewport");
									ImGui::SameLine();

									float totalAvailableWidth = ImGui::GetContentRegionAvail().x;
									float itemWidth = (totalAvailableWidth - ImGui::GetStyle().ItemSpacing.x * 3) / 4;

									ImGui::PushItemWidth(itemWidth);
									if (ImGui::DragFloat("##X", &camera->viewport.x, 0.01f, 0.0f, 1.0f, "X:%.2f")) {} ImGui::SameLine();
									if (ImGui::DragFloat("##Y", &camera->viewport.y, 0.01f, 0.0f, 1.0f, "Y:%.2f")) {} ImGui::SameLine();
									if (ImGui::DragFloat("##W", &camera->viewport.width, 0.01f, 0.001f, 1.0f, "W:%.2f")) {} ImGui::SameLine();
									if (ImGui::DragFloat("##H", &camera->viewport.height, 0.01f, 0.001f, 1.0f, "H:%.2f")) {}
									ImGui::PopItemWidth();

									ImGui::TreePop();
								}
							}

							if (Scriptable* scriptable = scene->GetECS().GetComponent<Scriptable>(gameObjectId))
							{
								if (DrawComponentHeader("Scriptable"))
								{
									ImGui::Text("Script Count: %llu", scriptable->_scripts.size());
									ImGui::SameLine();
									if (ImGui::Button("Clear All Scripts"))
									{
										scriptable->_scripts.clear();
									}

									ImGui::Separator();

									for (long long i = scriptable->_scripts.size() - 1; i >= 0; --i)
									{
										const auto& scriptPtr = scriptable->_scripts[i];
										char label[64];
										snprintf(label, sizeof(label), "Script %llu", i);
										ImGui::PushID((int)i);
										ImGui::Text("Script Index: %llu", i);
										ImGui::SameLine();

										if (ImGui::Button("Delete"))
										{
											scriptable->_scripts.erase(scriptable->_scripts.begin() + i);
											ImGui::PopID();
											break;
										}

										ImGui::PopID();
									}

									ImGui::TreePop();
								}
							}

							ImGui::Unindent();
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			}
		}
	}

	void DebugLayer::_DrawEventLogPanel()
	{
		if (ImGui::CollapsingHeader("Event Log"))
		{
			if (ImGui::Button("Clear Log"))
			{
				_eventLog.clear();
			}

			ImGui::BeginChild("EventScrollRegion");

			for (const auto& entry : _eventLog)
			{
				ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
				if (entry.type == EventType::WindowClose) color = ImVec4(1.0f, 0.5f, 0.5f, 1.0f);
				else if (entry.type == EventType::WindowResize) color = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);

				ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::Text("[%0.3f] [%d] %s", entry.timestamp, (int)entry.type, entry.message.c_str());
				ImGui::PopStyleColor();
			}

			ImGui::EndChild();
		}
	}

	void DebugLayer::_DrawInputPanel()
	{
		bool isCursorVisible = Input::GetMouse().IsCursorVisible();

		if (ImGui::CollapsingHeader("Input"))
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
						case Mouse::ButtonState::Released: return "Released";
						case Mouse::ButtonState::Hold:     return "Hold";
						case Mouse::ButtonState::Pressed:  return "Pressed (Down)";
						case Mouse::ButtonState::Up:       return "Up (Just Released)";
						default:                           return "Unknown";
						}
					};

				auto GetButtonName = [](Mouse::MouseBoutton button) -> const char*
					{
						switch (button)
						{
						case Mouse::MouseBoutton::Left:    return "Left";
						case Mouse::MouseBoutton::Middle:  return "Middle";
						case Mouse::MouseBoutton::Right:   return "Right";
						case Mouse::MouseBoutton::XButton1: return "XButton1";
						case Mouse::MouseBoutton::XButton2: return "XButton2";
						case Mouse::MouseBoutton::Count:   return "Count"; // Should not happen
						default:                           return "???";
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
		}
	}

	void DebugLayer::_DrawPerformancePanel()
	{
		if (ImGui::CollapsingHeader("Performance"))
		{
			float currentFrameTime = _frameTimes[(_frameTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];
			float fps = (currentFrameTime > 0.0f) ? (1000.0f / currentFrameTime) : 0.0f;

			ImGui::Text("Frame Time (Total): %.2f ms (FPS: %.0f)", currentFrameTime, fps);

			float averageFrameTime = 0.0f;
			for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i) averageFrameTime += _frameTimes[i];
			averageFrameTime /= FRAME_TIME_HISTORY_SIZE;
			std::string overlayFrame = "Avg: " + std::to_string(static_cast<int>(averageFrameTime)) + "ms | Max: " + std::to_string(static_cast<int>(_maxFrameTime)) + "ms";

			ImGui::PlotLines("Frame Time (ms)", _frameTimes, FRAME_TIME_HISTORY_SIZE,
				_frameTimeHistoryIndex, overlayFrame.c_str(),
				0.0f, _maxFrameTime * 1.2f, ImVec2(0, 80.0f));

			ImGui::Separator();

			float currentFixedTime = _fixedUpdateTimes[(_fixedUpdateTimeHistoryIndex - 1 + FRAME_TIME_HISTORY_SIZE) % FRAME_TIME_HISTORY_SIZE];

			ImGui::Text("Physics Update: %.2f ms", currentFixedTime);

			float averageFixedUpdateTime = 0.0f;
			for (int i = 0; i < FRAME_TIME_HISTORY_SIZE; ++i)
			{
				averageFixedUpdateTime += _fixedUpdateTimes[i];
			}
			averageFixedUpdateTime /= FRAME_TIME_HISTORY_SIZE;

			std::string overlayFixed = "Avg: " + std::to_string(static_cast<int>(averageFixedUpdateTime)) + "ms | Max: " + std::to_string(static_cast<int>(_maxFixedUpdateTime)) + "ms";

			ImGui::PlotLines(
				"Physics Time (ms)",
				_fixedUpdateTimes,
				FRAME_TIME_HISTORY_SIZE,
				_fixedUpdateTimeHistoryIndex,
				overlayFixed.c_str(),
				0.0f,
				_maxFixedUpdateTime * 1.2f,
				ImVec2(0, 80.0f)
			);
		}
	}

	void DebugLayer::_LogEvent(const Event& e, const std::string& message)
	{
		using float_seconds = std::chrono::duration<float>;
		float currentTime = _logTimer.GetDurationAs<float_seconds>().count();
		
		if (_eventLog.size() >= MAX_LOG_ENTRIES)
		{
			_eventLog.pop_front();
		}

		_eventLog.emplace_back(LogEntry{ currentTime, message, e.GetEventType() });
	}

	bool DebugLayer::_OnDebugOptionChanged(DebugOptionChangedEvent& e)
	{
		std::stringstream ss;
		ss << "Option: " << (int)e.GetOptionType() << " changed.";
		_LogEvent(e, ss.str());
		return false; // Don't handle the event, just log it
	}

	bool DebugLayer::_OnWindowClose(WindowCloseEvent& e)
	{
		_LogEvent(e, "Window received close request.");
		return false;
	}

	bool DebugLayer::_OnWindowResize(WindowResizeEvent& e)
	{
		std::stringstream ss;
		ss << "Window resized to " << e.GetWidth() << "x" << e.GetHeight() << ".";
		_LogEvent(e, ss.str());
		return false;
	}
}