#include "Editor/ProjectHubLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/UI/EditorTheme.h"
#include "Editor/Project/ProjectOpenEvent.h"
#include "Editor/Utils/PlatformUtils.h"
#include "Frost.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <string>
#include <cstring>

using namespace Frost;

namespace Editor
{
	namespace fs = std::filesystem;

	ProjectHubLayer::ProjectHubLayer() : Frost::Layer(GetStaticName())
	{
		std::string defaultPath = fs::current_path().string();
		strncpy_s(_newProjectLocationBuffer, defaultPath.c_str(), sizeof(_newProjectLocationBuffer));
	}

	void ProjectHubLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		EditorTheme::Apply();

		WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
		RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());

		ImGui_ImplWin32_Init(window->GetWindowHandle());
		ImGui_ImplDX11_Init(renderer->GetDevice(), renderer->GetDeviceContext());

		_LoadRecents();
	}

	void ProjectHubLayer::OnDetach()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ProjectHubLayer::OnLateUpdate(float deltaTime)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		_RenderHubUI();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void ProjectHubLayer::_RenderHubUI()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

		if (ImGui::Begin("ProjectHub", nullptr, flags))
		{
			// Logo
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
			float textWidth = ImGui::CalcTextSize("FROST ENGINE").x;
			ImGui::SetCursorPosX((ImGui::GetWindowWidth() - textWidth) * 0.5f);
			ImGui::Text("FROST ENGINE");
			ImGui::PopFont();
			ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

			ImGui::Columns(2, "HubColumns", false);
			ImGui::SetColumnWidth(0, 300.0f);

			// Left panel (Actions)
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 15));

			if (ImGui::Button("New project", ImVec2(-1, 50)))
			{
				ImGui::OpenPopup("Create New Project");
				_showCreateModal = true;
			}

			ImGui::Spacing();

			if (ImGui::Button("Open project", ImVec2(-1, 50)))
			{
				std::string filepath = FileDialogs::OpenFile("Frost Project (*.frost)\0*.frost\0");
				if (!filepath.empty())
				{
					_OpenProject(filepath);
				}
			}

			ImGui::PopStyleVar();

			float avail = ImGui::GetContentRegionAvail().y;
			ImGui::Dummy(ImVec2(0, avail - 50.0f));

			if (ImGui::Button("Exit", ImVec2(-1, 40)))
			{
				Frost::EventManager::Emit<WindowCloseEvent>();
			}

			ImGui::NextColumn();

			// Right panel (Recents)
			ImGui::TextDisabled("Recent projects");
			ImGui::Separator();

			ImGui::BeginChild("RecentsList");
			for (int i = 0; i < _recentProjects.size(); i++)
			{
				const std::string& path = _recentProjects[i];
				std::string projectName = fs::path(path).parent_path().filename().string();
				if (projectName.empty()) projectName = fs::path(path).filename().string();

				ImGui::PushID(i);

				if (ImGui::Selectable("##RecentItem", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 55)))
				{
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						_OpenProject(path);
					}
				}

				ImGui::SameLine();
				ImGui::BeginGroup();
				ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", projectName.c_str());
				ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "%s", path.c_str());
				ImGui::EndGroup();

				// Context Menu
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Open Project"))
					{
						_OpenProject(path);
					}
					if (ImGui::MenuItem("Reveal in Explorer"))
					{
						FileDialogs::OpenInExplorer(path);
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Remove from list"))
					{
						_recentProjects.erase(_recentProjects.begin() + i);
						_SaveRecents();
						ImGui::CloseCurrentPopup();
						ImGui::EndPopup();
						ImGui::PopID();
						i--;
						continue;
					}
					ImGui::EndPopup();
				}

				// Tooltip for long paths
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
				{
					ImGui::SetTooltip("%s", path.c_str());
				}

				ImGui::Separator();
				ImGui::PopID();
			}

			if (_recentProjects.empty())
			{
				const char* textLine1 = "No recent projects found.";
				const char* textLine2 = "Create or open one to get started.";

				float windowWidth = ImGui::GetWindowSize().x;
				float windowHeight = ImGui::GetWindowSize().y;

				float textWidth1 = ImGui::CalcTextSize(textLine1).x;
				float textWidth2 = ImGui::CalcTextSize(textLine2).x;
				float lineHeight = ImGui::GetTextLineHeight();
				float totalTextHeight = (lineHeight * 2) + ImGui::GetStyle().ItemSpacing.y;

				ImGui::SetCursorPosY((windowHeight - totalTextHeight) * 0.5f);

				ImGui::SetCursorPosX((windowWidth - textWidth1) * 0.5f);
				ImGui::TextDisabled("%s", textLine1);

				ImGui::SetCursorPosX((windowWidth - textWidth2) * 0.5f);
				ImGui::TextDisabled("%s", textLine2);
			}

			ImGui::EndChild();
			ImGui::Columns(1);

			_RenderCreateProjectPopup();

			ImGui::End();
		}
	}

	void ProjectHubLayer::_RenderCreateProjectPopup()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

		if (ImGui::BeginPopupModal("Create New Project", &_showCreateModal))
		{
			ImGui::Text("Configuration");
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 10));

			// Name input
			ImGui::Text("Project Name");
			ImGui::SetNextItemWidth(-1);
			ImGui::InputText("##PrjName", _newProjectNameBuffer, sizeof(_newProjectNameBuffer));

			ImGui::Dummy(ImVec2(0, 10));


			// Path input
			ImGui::Text("Location");
			ImGui::SetNextItemWidth(-40);
			ImGui::InputText("##PrjLoc", _newProjectLocationBuffer, sizeof(_newProjectLocationBuffer));
			ImGui::SameLine();

			if (ImGui::Button("...", ImVec2(32, 0)))
			{
				std::string folder = FileDialogs::PickFolder(_newProjectLocationBuffer);
				if (!folder.empty())
				{
					strncpy_s(_newProjectLocationBuffer, folder.c_str(), sizeof(_newProjectLocationBuffer));
				}
			}

			ImGui::Dummy(ImVec2(0, 5));
			fs::path fullPath = fs::path(_newProjectLocationBuffer) / _newProjectNameBuffer;
			ImGui::TextDisabled("Path: %s", fullPath.string().c_str());

			ImGui::Dummy(ImVec2(0, 20));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0, 10));

			if (ImGui::Button("Create Project", ImVec2(140, 35)))
			{
				if (strlen(_newProjectNameBuffer) == 0)
				{
					_ShowError("Project name cannot be empty.");
				}
				else if (strlen(_newProjectLocationBuffer) == 0)
				{
					_ShowError("Project location cannot be empty.");
				}
				else
				{
					std::string templatePath = R"(resources/projects/templates/DefaultTemplate)";

					if (ProjectInfo::CreateNewProject(_newProjectNameBuffer, _newProjectLocationBuffer, templatePath))
					{
						std::string finalPath = (fullPath / "project.frost").string();
						_OpenProject(finalPath);
						ImGui::CloseCurrentPopup();
						_showCreateModal = false;
					}
					else
					{
						_ShowError("Failed to create project.\nCheck console for details or ensure folder is empty.");
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(100, 35)))
			{
				ImGui::CloseCurrentPopup();
				_showCreateModal = false;
			}

			ImGui::EndPopup();
		}
	}

	void ProjectHubLayer::_OpenProject(const std::string& path)
	{
		if (!fs::exists(path))
		{
			auto it = std::find(_recentProjects.begin(), _recentProjects.end(), path);
			if (it != _recentProjects.end())
			{
				_recentProjects.erase(it);
				_SaveRecents();
			}
			return;
		}

		_AddToRecents(path);
		Frost::EventManager::Emit<ProjectOpenEvent>(path);
	}

	void ProjectHubLayer::_AddToRecents(const std::string& path)
	{
		auto it = std::find(_recentProjects.begin(), _recentProjects.end(), path);
		if (it != _recentProjects.end())
		{
			_recentProjects.erase(it);
		}

		_recentProjects.insert(_recentProjects.begin(), path);

		// Limit to 10 recent projects
		if (_recentProjects.size() > 10)
		{
			_recentProjects.resize(10);
		}

		_SaveRecents();
	}

	void ProjectHubLayer::_LoadRecents()
	{
		_recentProjects.clear();
		if (!fs::exists(_recentsFilePath))
		{
			return;
		}

		try
		{
			YAML::Node data = YAML::LoadFile(_recentsFilePath);
			if (data["Recents"])
			{
				for (auto item : data["Recents"])
				{
					std::string path = item.as<std::string>();
					if (fs::exists(path))
					{
						_recentProjects.push_back(path);
					}
				}
			}
		}
		catch (...)
		{
			FT_ENGINE_CRITICAL("Failed to load recent projects from '{}'.", _recentsFilePath);
		}
	}

	void ProjectHubLayer::_SaveRecents()
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Recents" << YAML::Value << YAML::BeginSeq;

		for (const auto& path : _recentProjects)
		{
			out << path;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(_recentsFilePath);
		fout << out.c_str();
	}

	void ProjectHubLayer::_ShowError(const std::string& msg)
	{
		_errorMessage = msg;
		_showErrorModal = true;
	}

	void ProjectHubLayer::_RenderErrorPopup()
	{
		if (_showErrorModal)
		{
			ImGui::OpenPopup("Error##HubError");
		}

		// Center this popup
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 center = viewport->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Error##HubError", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
			ImGui::TextWrapped("Error Occurred:");
			ImGui::PopStyleColor();

			ImGui::Spacing();
			ImGui::TextWrapped("%s", _errorMessage.c_str());
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				_showErrorModal = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}