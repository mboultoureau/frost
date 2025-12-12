#include "Editor/UI/MainMenuBar.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowCloseEvent.h"
#include "Editor/UI/ProjectSettingsWindow.h"
#include "Editor/Events/ProjectCloseEvent.h"
#include "Editor/Events/OpenProjectSettingsEvent.h"
#include "Editor/Events/NewSceneEvent.h"

#include "Frost/Scene/Scene.h"
#include "Frost/Scene/SceneSerializer.h"
#include "Frost/Scene/PrefabSerializer.h"
#include "Frost/Debugging/Logger.h"

#include <imgui.h>
#include <filesystem>

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
            _RenderBuildMenu();
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

    void MainMenuBar::_RenderBuildMenu()
    {
        if (ImGui::BeginMenu("Build"))
        {
            if (ImGui::MenuItem("Compile All Assets"))
            {
                _CompileAllAssets();
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

    void MainMenuBar::_CompileAllAssets()
    {
        std::filesystem::path projectDir = _projectInfo.GetProjectDir();
        if (!std::filesystem::exists(projectDir))
            return;

        FT_ENGINE_INFO("Starting asset compilation...");

        int sceneCount = 0;
        int prefabCount = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(projectDir))
        {
            if (entry.is_directory())
                continue;

            auto path = entry.path();
            auto extension = path.extension();

            if (extension == ".scene")
            {
                Frost::Scene tempScene;
                Frost::SceneSerializer serializer(&tempScene);
                if (serializer.Deserialize(path))
                {
                    std::filesystem::path binPath = path;
                    binPath.replace_extension(".bin");
                    if (serializer.Serialize(binPath))
                    {
                        sceneCount++;
                    }
                }
            }
            else if (extension == ".prefab")
            {
                Frost::Scene tempScene;
                Frost::GameObject root = Frost::PrefabSerializer::Instantiate(&tempScene, path);
                if (root)
                {
                    Frost::PrefabSerializer::CreatePrefab(root, path);
                    prefabCount++;
                }
            }
        }

        FT_ENGINE_INFO("Asset compilation finished. Scenes: {0}, Prefabs: {1}", sceneCount, prefabCount);
    }
} // namespace Editor