#include "Editor/UI/ProjectSettingsWindow.h"
#include "Editor/Utils/PhysicsCodeGenerator.h"

#include "Frost/Debugging/Logger.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

using namespace Frost;

namespace Editor
{
    ProjectSettingsWindow::ProjectSettingsWindow(ProjectInfo& projectInfo) :
        _projectInfo(projectInfo), _editableConfig(projectInfo.GetConfig())
    {
    }

    void ProjectSettingsWindow::Draw(float deltaTime)
    {
        if (!_isOpen)
            return;

        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin(GetStaticTitle(), &_isOpen, ImGuiWindowFlags_NoCollapse))
        {
            // Top action bar
            if (ImGui::Button("Apply"))
            {
                _projectInfo.GetConfigRef() = _editableConfig;
                _projectInfo.Save();

                std::string outputPath = _projectInfo.GetProjectDir() + "/src/Physics";

                if (Editor::PhysicsCodeGenerator::Generate(_projectInfo.GetConfig(), outputPath))
                {
                    ImGui::OpenPopup("SuccessGeneration");
                }
                else
                {
                    FT_ENGINE_ERROR("Unable to generate physics layer code.");
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                _editableConfig = _projectInfo.GetConfig();
            }

            ImGui::Separator();

            ImGui::BeginChild("LeftPane", ImVec2(150, 0), true);
            _RenderLeftPane();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("RightPane", ImVec2(0, 0), true);
            _RenderRightPane();
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void ProjectSettingsWindow::_RenderLeftPane()
    {
        if (ImGui::Selectable("General", _currentCategory == ESettingsCategory::General))
        {
            _currentCategory = ESettingsCategory::General;
        }
        if (ImGui::Selectable("Physics", _currentCategory == ESettingsCategory::Physics))
        {
            _currentCategory = ESettingsCategory::Physics;
        }
    }

    void ProjectSettingsWindow::_RenderRightPane()
    {
        switch (_currentCategory)
        {
            case ESettingsCategory::General:
                _RenderGeneralSettings();
                break;
            case ESettingsCategory::Physics:
                _RenderPhysicsSettings();
                break;
        }
    }

    void ProjectSettingsWindow::_RenderGeneralSettings()
    {
        ImGui::Text("Project Name");
        ImGui::InputText("##ProjectName", &_editableConfig.name);

        ImGui::Text("Start Scene");
        ImGui::InputText("##StartScene", &_editableConfig.startScene);
    }

    void ProjectSettingsWindow::_RenderPhysicsSettings()
    {
        if (ImGui::CollapsingHeader("BroadPhase Layers", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (int i = 0; i < _editableConfig.broadPhaseLayers.size(); ++i)
            {
                ImGui::PushID(i);

                // Minimum one layer must exist
                if (_editableConfig.broadPhaseLayers.size() > 1)
                {
                    if (ImGui::Button("-"))
                    {
                        _RemoveBroadPhaseLayer(i);
                        ImGui::PopID();
                        continue;
                    }
                    ImGui::SameLine();
                }

                ImGui::SetNextItemWidth(200.0f);
                ImGui::InputText("##BPName", &_editableConfig.broadPhaseLayers[i].name);
                ImGui::SameLine();
                ImGui::Text("ID: %d", _editableConfig.broadPhaseLayers[i].layerId);

                ImGui::PopID();
            }

            if (ImGui::Button("+##AddBroadPhaseLayer"))
            {
                _AddBroadPhaseLayer();
            }
        }

        if (ImGui::CollapsingHeader("Object Layers", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // At least one BroadPhase layer must exist to create Object layers
            if (_editableConfig.broadPhaseLayers.empty())
            {
                ImGui::TextDisabled("Please create a BroadPhase Layer first.");
            }
            else
            {
                for (int i = 0; i < _editableConfig.objectLayers.size(); ++i)
                {
                    ImGui::PushID(i + 1000);

                    if (ImGui::Button("-"))
                    {
                        _RemoveObjectLayer(i);
                        ImGui::PopID();
                        continue;
                    }
                    ImGui::SameLine();

                    ImGui::SetNextItemWidth(200.0f);
                    ImGui::InputText("##OLName", &_editableConfig.objectLayers[i].name);
                    ImGui::SameLine();

                    const char* currentBPName = "None";
                    auto it =
                        std::find_if(_editableConfig.broadPhaseLayers.begin(),
                                     _editableConfig.broadPhaseLayers.end(),
                                     [&](const BroadPhaseLayerSetting& bpLayer)
                                     { return bpLayer.layerId == _editableConfig.objectLayers[i].broadPhaseLayerId; });
                    if (it != _editableConfig.broadPhaseLayers.end())
                    {
                        currentBPName = it->name.c_str();
                    }

                    if (ImGui::BeginCombo("BroadPhase", currentBPName))
                    {
                        for (const auto& bpLayer : _editableConfig.broadPhaseLayers)
                        {
                            const bool isSelected =
                                (bpLayer.layerId == _editableConfig.objectLayers[i].broadPhaseLayerId);
                            if (ImGui::Selectable(bpLayer.name.c_str(), isSelected))
                            {
                                _editableConfig.objectLayers[i].broadPhaseLayerId = bpLayer.layerId;
                            }
                            if (isSelected)
                            {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::SameLine();
                    ImGui::Text("ID: %d", _editableConfig.objectLayers[i].layerId);

                    ImGui::PopID();
                }

                if (ImGui::Button("+##AddObjectLayer"))
                {
                    _AddObjectLayer();
                }
            }
        }

        if (ImGui::CollapsingHeader("BroadPhase Collision Matrix", ImGuiTreeNodeFlags_DefaultOpen))
        {
            _RenderCollisionMatrix(_editableConfig.broadPhaseLayers, _editableConfig.broadPhaseCollisionMatrix);
        }

        if (ImGui::CollapsingHeader("Object Collision Matrix", ImGuiTreeNodeFlags_DefaultOpen))
        {
            _RenderCollisionMatrix(_editableConfig.objectLayers, _editableConfig.objectCollisionMatrix);
        }

        if (ImGui::CollapsingHeader("Object vs BroadPhase Collision", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int numObjectLayers = _editableConfig.objectLayers.size();
            int numBroadPhaseLayers = _editableConfig.broadPhaseLayers.size();

            if (_editableConfig.objectBroadPhaseCollisionMatrix.size() != numObjectLayers * numBroadPhaseLayers)
            {
                ImGui::Text("Matrix size mismatch!");
            }

            if (ImGui::BeginTable("ObjectVsBroadPhaseMatrix", numBroadPhaseLayers + 1))
            {
                // Header
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("");
                for (int i = 0; i < numBroadPhaseLayers; ++i)
                {
                    ImGui::TableSetColumnIndex(i + 1);
                    ImGui::TextUnformatted(_editableConfig.broadPhaseLayers[i].name.c_str());
                }

                // Rows
                for (int i = 0; i < numObjectLayers; ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(_editableConfig.objectLayers[i].name.c_str());

                    for (int j = 0; j < numBroadPhaseLayers; ++j)
                    {
                        ImGui::TableSetColumnIndex(j + 1);
                        int index = i * numBroadPhaseLayers + j;

                        bool isChecked = _editableConfig.objectBroadPhaseCollisionMatrix[index];
                        if (ImGui::Checkbox(std::string("##OVB" + std::to_string(index)).c_str(), &isChecked))
                        {
                            _editableConfig.objectBroadPhaseCollisionMatrix[index] = isChecked;
                        }
                    }
                }

                ImGui::EndTable();
            }
        }
    }

    template<typename TLayer>
    void ProjectSettingsWindow::_RenderCollisionMatrix(const std::vector<TLayer>& layers, std::vector<bool>& matrix)
    {
        int numLayers = layers.size();
        if (numLayers == 0)
            return;

        if (ImGui::BeginTable("CollisionMatrix", numLayers + 1))
        {
            // Header row
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted("");
            for (int i = 0; i < numLayers; ++i)
            {
                ImGui::TableSetColumnIndex(i + 1);
                ImGui::TextUnformatted(layers[i].name.c_str());
            }

            // Matrix rows
            for (int i = 0; i < numLayers; ++i)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(layers[i].name.c_str());

                for (int j = 0; j < numLayers; ++j)
                {
                    ImGui::TableSetColumnIndex(j + 1);

                    if (j < i)
                    {
                        ImGui::TextUnformatted("");
                    }
                    else
                    {
                        int index1 = i * numLayers + j;
                        int index2 = j * numLayers + i;
                        bool val = matrix[index1];
                        if (ImGui::Checkbox(std::string("##" + std::to_string(index1)).c_str(), &val))
                        {
                            matrix[index1] = val;
                            matrix[index2] = val;
                        }
                    }
                }
            }
            ImGui::EndTable();
        }
    }

    void ProjectSettingsWindow::_AddBroadPhaseLayer()
    {
        uint8_t newId = 0;
        while (true)
        {
            auto it = std::find_if(_editableConfig.broadPhaseLayers.begin(),
                                   _editableConfig.broadPhaseLayers.end(),
                                   [newId](const BroadPhaseLayerSetting& l) { return l.layerId == newId; });
            if (it == _editableConfig.broadPhaseLayers.end())
            {
                break;
            }
            newId++;
            if (newId > 254)
            {
                return;
            }
        }

        _editableConfig.broadPhaseLayers.push_back({ "New BroadPhase Layer", newId });

        int newSize = _editableConfig.broadPhaseLayers.size();
        _editableConfig.broadPhaseCollisionMatrix.resize(newSize * newSize, false);
        _editableConfig.objectBroadPhaseCollisionMatrix.resize(_editableConfig.objectLayers.size() * newSize, false);
    }

    void ProjectSettingsWindow::_RemoveBroadPhaseLayer(int indexToRemove)
    {
        if (indexToRemove < 0 || indexToRemove >= _editableConfig.broadPhaseLayers.size())
            return;
        if (_editableConfig.broadPhaseLayers.size() <= 1)
            return;

        uint8_t removedId = _editableConfig.broadPhaseLayers[indexToRemove].layerId;

        _editableConfig.broadPhaseLayers.erase(_editableConfig.broadPhaseLayers.begin() + indexToRemove);

        uint8_t fallbackId = _editableConfig.broadPhaseLayers[0].layerId;
        for (auto& objectLayer : _editableConfig.objectLayers)
        {
            if (objectLayer.broadPhaseLayerId == removedId)
            {
                objectLayer.broadPhaseLayerId = fallbackId;
            }
        }

        int newSize = _editableConfig.broadPhaseLayers.size();
        _editableConfig.broadPhaseCollisionMatrix.resize(newSize * newSize, false);
        _editableConfig.objectBroadPhaseCollisionMatrix.resize(_editableConfig.objectLayers.size() * newSize, false);
    }

    void ProjectSettingsWindow::_AddObjectLayer()
    {
        if (_editableConfig.broadPhaseLayers.empty())
            return;

        JPH::ObjectLayer newId = 0;
        while (true)
        {
            auto it = std::find_if(_editableConfig.objectLayers.begin(),
                                   _editableConfig.objectLayers.end(),
                                   [newId](const ObjectLayerSetting& l) { return l.layerId == newId; });
            if (it == _editableConfig.objectLayers.end())
            {
                break;
            }
            newId++;
        }

        _editableConfig.objectLayers.push_back(
            { "New Object Layer", newId, _editableConfig.broadPhaseLayers[0].layerId });

        int newSize = _editableConfig.objectLayers.size();
        _editableConfig.objectCollisionMatrix.resize(newSize * newSize, false);
        _editableConfig.objectBroadPhaseCollisionMatrix.resize(newSize * _editableConfig.broadPhaseLayers.size(),
                                                               false);
    }

    void ProjectSettingsWindow::_RemoveObjectLayer(int indexToRemove)
    {
        if (indexToRemove < 0 || indexToRemove >= _editableConfig.objectLayers.size())
            return;

        _editableConfig.objectLayers.erase(_editableConfig.objectLayers.begin() + indexToRemove);

        int newSize = _editableConfig.objectLayers.size();
        _editableConfig.objectCollisionMatrix.resize(newSize * newSize, false);
        _editableConfig.objectBroadPhaseCollisionMatrix.resize(newSize * _editableConfig.broadPhaseLayers.size(),
                                                               false);
    }
} // namespace Editor