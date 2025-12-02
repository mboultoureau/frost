#include "Frost/Debugging/DebugInterface/DebugUtils.h"
#include "Frost/Utils/Math/Angle.h"

#include <imgui.h>
#include <imgui_internal.h>

using namespace Frost::Math;

namespace Frost
{
    bool DebugUtils::DrawComponentHeader(const char* name, bool* outRemoved)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx(name,
                                      ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
                                          ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_DefaultOpen);
        ImGui::PopStyleVar();

        if (outRemoved)
        {
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Remove Component"))
                {
                    *outRemoved = true;
                }
                ImGui::EndPopup();
            }
        }

        return open;
    }

    bool DebugUtils::DrawVec3Control(const std::string& label,
                                     Math::Vector3& values,
                                     float resetValue,
                                     float columnWidth)
    {
        // Display values
        Math::Vector3 displayValues = values;
        std::string format = "%.2f";

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;

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
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            displayValues.x = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &displayValues.x, 0.1f, 0.0f, 0.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // --- Y ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            displayValues.y = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &displayValues.y, 0.1f, 0.0f, 0.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // --- Z ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            displayValues.z = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Z", &displayValues.z, 0.1f, 0.0f, 0.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();

        if (modified)
        {
            values = displayValues;
        }

        return modified;
    }

    bool DebugUtils::DrawQuaternionControl(const std::string& label,
                                           Math::Vector4& quaternion,
                                           float resetValue,
                                           float columnWidth)
    {
        EulerAngles eulerRadians = QuaternionToEulerAngles(quaternion);
        Vector3 eulerDegrees = { Angle<Degree>(eulerRadians.Pitch).value(),
                                 Angle<Degree>(eulerRadians.Yaw).value(),
                                 Angle<Degree>(eulerRadians.Roll).value() };

        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts.Size > 0 ? io.Fonts->Fonts[0] : nullptr;

        bool modified = false;
        std::string format = "%.2f deg";

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetFontSize() + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // --- X (Pitch) ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
        {
            eulerDegrees.x = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##X", &eulerDegrees.x, 0.5f, -180.0f, 180.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // --- Y (Yaw) ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
        {
            eulerDegrees.y = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Y", &eulerDegrees.y, 0.5f, -180.0f, 180.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // --- Z (Roll) ---
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (boldFont)
            ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
        {
            eulerDegrees.z = resetValue;
            modified = true;
        }
        if (boldFont)
            ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if (ImGui::DragFloat("##Z", &eulerDegrees.z, 0.5f, -180.0f, 180.0f, format.c_str()))
            modified = true;
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();

        if (modified)
        {
            // Convert degrees back to radians
            eulerRadians.Pitch = Angle<Degree>(eulerDegrees.x);
            eulerRadians.Yaw = Angle<Degree>(eulerDegrees.y);
            eulerRadians.Roll = Angle<Degree>(eulerDegrees.z);

            // Convert Euler radians back to Quaternion
            quaternion = EulerToQuaternion(eulerRadians);
        }

        return modified;
    }
} // namespace Frost
