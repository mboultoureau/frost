#pragma once

#include "Frost.h"

#include "Editor/UI/ProjectSettingsWindow.h"
#include "Editor/UI/ContentBrowser/ContentBrowser.h"
#include "Editor/UI/MainMenuBar.h"
#include "Editor/UI/Scene/SceneView.h"
#include "Editor/UI/StatusBar.h"
#include "Editor/Project/ProjectInfo.h"
#include "Editor/Project/OpenProjectSettingsEvent.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Scene/Scene.h"

#include <imgui.h>

namespace Editor
{
    class EditorLayer : public Frost::Layer
    {
    public:
        EditorLayer();

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(float deltaTime) override;
        void OnLateUpdate(float deltaTime) override;
        void OnFixedUpdate(float fixedDeltaTime) override;

        static LayerName GetStaticName() { return "EditorLayer"; }

        void OpenPrefab(const std::filesystem::path& path);
        void OpenMeshPreview(const std::filesystem::path& path);
        static EditorLayer& Get();

    private:
        void _RenderUI(float deltaTime);
        bool _OnOpenProjectSettings(const OpenProjectSettingsEvent& e);

    private:
        static EditorLayer* _instance;

        Frost::EventHandlerId _openProjectSettingsHandlerId;

        std::unique_ptr<ProjectSettingsWindow> _projectSettingsWindow;
        std::unique_ptr<MainMenuBar> _mainMenuBar;
        std::unique_ptr<ContentBrowser> _contentBrowser;
        std::unique_ptr<StatusBar> _statusBar;
        std::vector<std::unique_ptr<SceneView>> _views;

        bool _projectSettingsNeedsDocking = false;

        ProjectInfo _projectInfo;
        std::unique_ptr<Frost::Scene> _activeScene;
        SceneView* _activeSceneView = nullptr;

        ImGuiID _dockMainID = 0;
        ImGuiID _dockRightID = 0;
    };
} // namespace Editor
