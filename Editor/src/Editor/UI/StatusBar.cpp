#include "Editor/UI/StatusBar.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
    void StatusBar::Draw()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        float height = ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 2.0f;

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - height));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, height));

        viewport->WorkSize.y -= height;

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

        if (ImGui::Begin("StatusBar", nullptr, flags))
        {
            if (ImGui::Button("RECOMPILE SCRIPTS"))
            {
            }

            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();

            ImGui::Text("Status: Ready");

            ImGui::SameLine();
            float fps = ImGui::GetIO().Framerate;
            float textWidth = ImGui::CalcTextSize("FPS: 999.9").x;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textWidth - 10);
            ImGui::Text("FPS: %.1f", fps);
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
} // namespace Editor