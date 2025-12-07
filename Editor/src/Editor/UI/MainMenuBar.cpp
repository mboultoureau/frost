#include "Editor/UI/MainMenuBar.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Editor/UI/ProjectSettingsWindow.h"
#include "Editor/Events/ProjectCloseEvent.h"
#include "Editor/Events/OpenProjectSettingsEvent.h"
#include "Editor/Events/NewSceneEvent.h"

#include <imgui.h>

using namespace Frost;

namespace Editor
{
    MainMenuBar::MainMenuBar(const ProjectInfo& projectInfo) : _projectInfo(projectInfo) {}

    void MainMenuBar::Draw(float deltaTime)
    {
        if (ImGui::BeginMainMenuBar())
        {
            _RenderFileMenu();
            _RenderEditMenu();
            _RenderProjectName();
            ImGui::EndMainMenuBar();
        }
    }

    void MainMenuBar::_RenderFileMenu()
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N"))
            {
                EventManager::Emit<NewSceneEvent>();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Close Project"))
            {
                EventManager::Emit<ProjectCloseEvent>();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Exit"))
            {
                EventManager::Emit<WindowCloseEvent>();
            }
            ImGui::EndMenu();
        }
    }

    void MainMenuBar::_RenderEditMenu()
    {
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Project Settings..."))
            {
                EventManager::Emit<OpenProjectSettingsEvent>();
            }
            ImGui::EndMenu();
        }
    }

    void MainMenuBar::_RenderProjectName()
    {
        std::string projectName = _projectInfo.GetConfig().name;

        float widthRight = ImGui::CalcTextSize(projectName.c_str()).x + 10.0;
        float avail = ImGui::GetContentRegionAvail().x;

        if (avail > widthRight)
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - widthRight);
        }

        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", projectName.c_str());
    }
} // namespace Editor