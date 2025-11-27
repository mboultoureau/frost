#include "ContentBrowser.h"

#include <imgui.h>

namespace Editor
{
    void ContentBrowser::Draw()
    {
        ImGui::Begin("Content Browser", &_isOpen);

        float padding = 16.0f;
        float thumbnailSize = 64.0f;
        float cellSize = thumbnailSize + padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int)(panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::Columns(columnCount, 0, false);

        const char* files[] = { "Player.fbx", "Enemy.obj", "Texture.png", "Level1.map", "Script.cpp" };

        for (int i = 0; i < 5; i++)
        {
            ImGui::PushID(i);
            ImGui::Button(files[i], ImVec2(thumbnailSize, thumbnailSize));

            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", files[i], strlen(files[i]) + 1);
                ImGui::Text("Moving %s", files[i]);
                ImGui::EndDragDropSource();
            }

            ImGui::TextWrapped("%s", files[i]);
            ImGui::NextColumn();
            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::End();
    }
} // namespace Editor