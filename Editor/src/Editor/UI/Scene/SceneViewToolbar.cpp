#include "Editor/UI/Scene/SceneViewToolbar.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Editor
{
    SceneViewToolbar::SceneViewToolbar() {}

    void SceneViewToolbar::Init()
    {
        _GetIcon("save");
        _GetIcon("mouse");
        _GetIcon("translate");
        _GetIcon("rotate");
        _GetIcon("scale");
        _GetIcon("camera_settings");
        _GetIcon("view_settings");
    }

    std::shared_ptr<Frost::Texture> SceneViewToolbar::_GetIcon(const std::string& name)
    {
        if (_icons.find(name) != _icons.end())
        {
            return _icons[name];
        }

        Frost::TextureConfig config;
        config.loadImmediately = true;
        config.path = "resources/editor/icons/" + name + ".png";

        auto texture = Frost::Texture::Create(config);
        if (texture)
        {
            _icons[name] = texture;
        }
        return texture;
    }

    bool SceneViewToolbar::_DrawToggleButton(const std::string& id, std::shared_ptr<Frost::Texture> icon, bool selected)
    {
        bool clicked = false;

        if (selected)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        }

        if (icon)
        {
            if (ImGui::ImageButton(id.c_str(), (ImTextureID)icon->GetRendererID(), ImVec2(_buttonSize, _buttonSize)))
            {
                clicked = true;
            }
        }
        else
        {
            if (ImGui::Button(id.c_str(), ImVec2(_buttonSize, _buttonSize)))
            {
                clicked = true;
            }
        }

        ImGui::PopStyleColor(selected ? 2 : 1);
        return clicked;
    }

    void SceneViewToolbar::Draw(EditorCameraController& cam,
                                GizmoOperation& currentGizmo,
                                SceneViewSettings& settings,
                                bool isPrefabView,
                                const std::function<void()>& onSavePrefabCallback,
                                const std::function<void()>& onSaveSceneCallback,
                                const std::function<void()>& onLoadSceneCallback)
    {
        // Handle inputs
        bool keyboardCapturedByWidget = ImGui::GetIO().WantCaptureKeyboard;

        if (!keyboardCapturedByWidget)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Q))
                currentGizmo = GizmoOperation::None;
            if (ImGui::IsKeyPressed(ImGuiKey_T))
                currentGizmo = GizmoOperation::Translate;
            if (ImGui::IsKeyPressed(ImGuiKey_E))
                currentGizmo = GizmoOperation::Rotate;
            if (ImGui::IsKeyPressed(ImGuiKey_R))
                currentGizmo = GizmoOperation::Scale;

            if (ImGui::IsKeyPressed(ImGuiKey_Space))
            {
                switch (currentGizmo)
                {
                    case GizmoOperation::None:
                        currentGizmo = GizmoOperation::Translate;
                        break;
                    case GizmoOperation::Translate:
                        currentGizmo = GizmoOperation::Rotate;
                        break;
                    case GizmoOperation::Rotate:
                        currentGizmo = GizmoOperation::Scale;
                        break;
                    case GizmoOperation::Scale:
                        currentGizmo = GizmoOperation::Translate;
                        break;
                }
            }
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

        // Save
        auto saveIcon = _GetIcon("save");
        if (isPrefabView)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::ImageButton("##SavePrefab",
                                   (ImTextureID)(saveIcon ? saveIcon->GetRendererID() : 0),
                                   ImVec2(_buttonSize, _buttonSize)))
            {
                if (onSavePrefabCallback)
                    onSavePrefabCallback();
            }
            ImGui::PopStyleColor();

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save Prefab (Ctrl+S)");
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::ImageButton("##SaveScene",
                                   (ImTextureID)(saveIcon ? saveIcon->GetRendererID() : 0),
                                   ImVec2(_buttonSize, _buttonSize)))
            {
                if (onSaveSceneCallback)
                    onSaveSceneCallback();
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Save Scene (Ctrl+S)");

            ImGui::SameLine();

            /*
            auto openIcon = _GetIcon("open");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            if (ImGui::ImageButton("##LoadScene",
                                   (ImTextureID)(openIcon ? openIcon->GetRendererID() : 0),
                                   ImVec2(_buttonSize, _buttonSize)))
            {
                if (onLoadSceneCallback)
                    onLoadSceneCallback();
            }
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Load Scene (Ctrl+O)");
                */
        }

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        // Gizmos
        if (_DrawToggleButton("##Mouse", _GetIcon("mouse"), currentGizmo == GizmoOperation::None))
        {
            currentGizmo = GizmoOperation::None;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Mouse (Q)");
        ImGui::SameLine();

        if (_DrawToggleButton("##Trans", _GetIcon("translate"), currentGizmo == GizmoOperation::Translate))
        {
            currentGizmo = GizmoOperation::Translate;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Translate (T)");
        ImGui::SameLine();

        if (_DrawToggleButton("##Rot", _GetIcon("rotate"), currentGizmo == GizmoOperation::Rotate))
        {
            currentGizmo = GizmoOperation::Rotate;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Rotate (E)");
        ImGui::SameLine();

        if (_DrawToggleButton("##Scale", _GetIcon("scale"), currentGizmo == GizmoOperation::Scale))
        {
            currentGizmo = GizmoOperation::Scale;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Scale (R)");

        float rightOffset = (_buttonSize + 16) * 2.0f;
        float avail = ImGui::GetContentRegionAvail().x;

        if (avail > rightOffset)
        {
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - rightOffset);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if (ImGui::ImageButton("##CamSet",
                               (ImTextureID)_GetIcon("camera_settings")->GetRendererID(),
                               ImVec2(_buttonSize, _buttonSize)))
        {
            ImGui::OpenPopup("CameraSettingsPopup");
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Camera Speed & Settings");

        if (ImGui::BeginPopup("CameraSettingsPopup"))
        {
            ImGui::Text("Camera Controller");
            ImGui::Separator();

            ImGui::DragFloat("Speed", &cam.moveSpeed, 0.1f, 0.1f, 100.0f);
            ImGui::DragFloat("Turbo Multiplier", &cam.turboMultiplier, 0.1f, 1.0f, 10.0f);
            ImGui::DragFloat("Sensitivity", &cam.mouseSensitivity, 0.0001f, 0.0001f, 0.01f, "%.4f");
            ImGui::DragFloat("Damping", &cam.damping, 0.1f, 0.0f, 20.0f);

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        // View Settings
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        if (ImGui::ImageButton(
                "##ViewOpt", (ImTextureID)_GetIcon("view_settings")->GetRendererID(), ImVec2(_buttonSize, _buttonSize)))
        {
            ImGui::OpenPopup("ViewSettingsPopup");
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("View Options");

        if (ImGui::BeginPopup("ViewSettingsPopup"))
        {
            ImGui::Text("Viewport Options");
            ImGui::Separator();

            ImGui::Checkbox("Show Editor Skybox", &settings.showEditorSkybox);
            ImGui::Checkbox("Show Editor Light", &settings.showEditorLight);

            ImGui::Separator();

            ImGui::Checkbox("Show Physics Debug", &Frost::Debug::PhysicsConfig::IsDisplayEnabled());
            // ImGui::Checkbox("Show Grid", &settings.showGrid);
            // ImGui::Checkbox("Wireframe Mode", &settings.showWireframe);
            // ImGui::Checkbox("Enable Lighting", &settings.enableLighting);
            // ImGui::Checkbox("Post Processing", &settings.showPostProcess);

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
    }
} // namespace Editor