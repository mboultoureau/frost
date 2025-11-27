#include "MainMenuBar.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"

#include <imgui.h>

using namespace Frost;

namespace Editor
{
    void MainMenuBar::Draw()
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
            // if (ImGui::MenuItem("New Scene")) {}
            // if (ImGui::MenuItem("Save", "Ctrl+S")) {}
            if (ImGui::MenuItem("Exit"))
            {
                EventManager::Emit<WindowCloseEvent>();
            }
            ImGui::EndMenu();
        }
    }

    void MainMenuBar::_RenderEditMenu()
    {
        /*
        if (ImGui::BeginMenu("Edit"))
        {
                //ImGui::MenuItem("Undo", "Ctrl+Z");
                ImGui::EndMenu();
        }
        */
    }

    void MainMenuBar::_RenderProjectName()
    {
        std::string ProjectName = "Game";

        float widthRight = ImGui::CalcTextSize(ProjectName.c_str()).x + 10.0;
        float avail = ImGui::GetContentRegionAvail().x;

        if (avail > widthRight)
        {
            ImGui::SameLine(ImGui::GetWindowWidth() - widthRight);
        }

        ImGui::TextColored(ImVec4(0.6f, 0.6f, 1.0f, 1.0f), "%s", ProjectName.c_str());
    }
} // namespace Editor