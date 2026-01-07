#include "Editor/EditorLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/UI/EditorTheme.h"
#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Scripting/ScriptingEngine.h"
#include "Frost/Scene/Systems/ScriptableSystem.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

using namespace Frost;
using namespace Frost::Scripting;

namespace Editor
{
    EditorLayer* EditorLayer::_instance = nullptr;

    EditorLayer::EditorLayer() : Frost::Layer(GetStaticName())
    {
        _instance = this;
    }

    void EditorLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Attach context to Frost
        ImGuiIO& io = ImGui::GetIO();
        ImGuiMemAllocFunc allocFunc;
        ImGuiMemFreeFunc freeFunc;
        void* userData;
        ImGui::GetAllocatorFunctions(&allocFunc, &freeFunc, &userData);
        Frost::SyncImGuiContext(ImGui::GetCurrentContext(), allocFunc, freeFunc, userData);

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch

        EditorTheme::Apply();

#ifdef FT_PLATFORM_WINDOWS
        WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());

        ImGui_ImplWin32_Init(window->GetWindowHandle());
        ImGui_ImplDX11_Init(renderer->GetDevice(), renderer->GetDeviceContext());
#else
#error "Platform not supported!"
#endif

        // Initialize events
        _openProjectSettingsHandlerId =
            EventManager::Subscribe<OpenProjectSettingsEvent>(FROST_BIND_EVENT_FN(EditorLayer::_OnOpenProjectSettings));
        _newSceneHandlerId = EventManager::Subscribe<NewSceneEvent>(FROST_BIND_EVENT_FN(EditorLayer::_OnNewScene));

        auto& app = EditorApp::Get();
        _projectInfo = app.GetProjectInfo();

        // Initialize Editor UI components
        _projectSettingsWindow = std::make_unique<ProjectSettingsWindow>(_projectInfo);
        _mainMenuBar = std::make_unique<MainMenuBar>(_projectInfo);
        _contentBrowser = std::make_unique<ContentBrowser>(_projectInfo);
        _statusBar = std::make_unique<StatusBar>();

        _SetupScriptingWatcher();
    }

    void EditorLayer::OnDetach()
    {
        _scriptingWatcher.Stop();

        EventManager::Unsubscribe<OpenProjectSettingsEvent>(_openProjectSettingsHandlerId);

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorLayer::OnUpdate(float deltaTime)
    {
        if (_scriptingWatcher.ReloadRequested())
        {
            TriggerHotReload();
        }
    }

    void EditorLayer::OnLateUpdate(float deltaTime)
    {
        for (auto it = _views.begin(); it != _views.end();)
        {
            if (!(*it)->IsOpen())
            {
                if (_activeSceneView == it->get())
                    _activeSceneView = nullptr;

                it = _views.erase(it);
            }
            else
            {
                (*it)->OnUpdate(deltaTime);

                if ((*it)->IsFocused())
                {
                    _activeSceneView = it->get();
                }
                ++it;
            }
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        _RenderUI(deltaTime);

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    void EditorLayer::OnFixedUpdate(float fixedDeltaTime)
    {
        for (auto it = _views.begin(); it != _views.end();)
        {
            if (!(*it)->IsOpen())
            {
                if (_activeSceneView == it->get())
                    _activeSceneView = nullptr;

                it = _views.erase(it);
            }
            else
            {
                (*it)->OnFixedUpdate(fixedDeltaTime);

                if ((*it)->IsFocused())
                {
                    _activeSceneView = it->get();
                }
                ++it;
            }
        }
    }

    EditorLayer& EditorLayer::Get()
    {
        FT_ENGINE_ASSERT(_instance, "EditorLayer instance is null!");
        return *_instance;
    }

    void EditorLayer::TriggerHotReload()
    {
        for (auto& view : _views)
        {
            auto& scene = view->GetScene();
            ScriptableSystem* scriptSystem = scene.GetSystem<ScriptableSystem>();

            if (scriptSystem)
            {
                scriptSystem->OnScriptsWillReload();
            }
        }

        std::filesystem::path projectDir = _projectInfo.GetProjectDir();
        std::filesystem::path dllPath = projectDir / _projectInfo.GetConfig().scriptingModule;

        ScriptingEngine::GetInstance().UnloadScriptingDLL();
        ScriptingEngine::GetInstance().LoadScriptingDLL(dllPath.string());

        for (auto& view : _views)
        {
            auto& scene = view->GetScene();
            ScriptableSystem* scriptSystem = scene.GetSystem<ScriptableSystem>();

            if (scriptSystem)
            {
                scriptSystem->OnScriptsReloaded();
            }
        }
    }

    void EditorLayer::OpenPrefab(const std::filesystem::path& path)
    {
        std::string titleToCheck = path.stem().string();
        for (const auto& view : _views)
        {
            if (view->GetTitle() == titleToCheck)
            {
                ImGui::SetWindowFocus(view->GetTitle().c_str());
                return;
            }
        }

        _views.push_back(std::make_unique<SceneView>(path));
        auto& newView = _views.back();

        if (_isLayoutInitialized && _dockMainID != 0)
        {
            ImGui::DockBuilderDockWindow(newView->GetTitle().c_str(), _dockMainID);
        }

        _activeSceneView = newView.get();
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path))
        {
            FT_ENGINE_ERROR("Cannot open scene: File does not exist at '{0}'", path.string());
            return;
        }

        std::string titleToCheck = path.stem().string();
        for (const auto& view : _views)
        {
            if (view->GetTitle() == titleToCheck)
            {
                ImGui::SetWindowFocus(view->GetTitle().c_str());
                return;
            }
        }

        auto newView = std::make_unique<SceneView>(path, SceneView::SceneTag{});

        if (_isLayoutInitialized && _dockMainID != 0)
        {
            ImGui::DockBuilderDockWindow(newView->GetTitle().c_str(), _dockMainID);
        }

        _views.push_back(std::move(newView));
    }

    void EditorLayer::OpenMeshPreview(const std::filesystem::path& path)
    {
        std::string titleToCheck = "Preview: " + path.filename().string();
        for (const auto& view : _views)
        {
            if (view->GetTitle() == titleToCheck)
            {
                ImGui::SetWindowFocus(view->GetTitle().c_str());
                return;
            }
        }

        _views.push_back(std::make_unique<SceneView>(path, SceneView::MeshPreviewTag{}));
        auto& newView = _views.back();
        if (_isLayoutInitialized && _dockMainID != 0)
        {
            ImGui::DockBuilderDockWindow(newView->GetTitle().c_str(), _dockMainID);
        }

        _activeSceneView = newView.get();
    }

    void EditorLayer::_RenderUI(float deltaTime)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspaceID = ImGui::GetID("DockSpace");

        _statusBar->Draw(deltaTime);

        ImGui::DockSpaceOverViewport(dockspaceID, viewport);

        if (!_isLayoutInitialized)
        {
            _isLayoutInitialized = true;

            ImGui::DockBuilderRemoveNode(dockspaceID);
            ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

            ImGuiID dock_main_id = dockspaceID;

            ImGuiID dock_right_id =
                ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);

            ImGuiID dock_right_down_id =
                ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.60f, nullptr, &dock_right_id);

            ImGuiID dock_bottom_id =
                ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.40f, nullptr, &dock_main_id);

            _dockMainID = dock_main_id;

            ImGui::DockBuilderDockWindow("Content Browser", dock_bottom_id);
            ImGui::DockBuilderDockWindow("Hierarchy", dock_right_id);
            ImGui::DockBuilderDockWindow("Inspector", dock_right_down_id);

            for (const auto& view : _views)
            {
                ImGui::DockBuilderDockWindow(view->GetTitle().c_str(), _dockMainID);
            }

            ImGui::DockBuilderFinish(dockspaceID);
        }

        if (_projectSettingsNeedsDocking)
        {
            ImGui::DockBuilderDockWindow(ProjectSettingsWindow::GetStaticTitle(), _dockMainID);
            _projectSettingsNeedsDocking = false;
        }

        _projectSettingsWindow->Draw(deltaTime);
        _mainMenuBar->Draw(deltaTime);
        _contentBrowser->Draw(deltaTime);

        for (auto& view : _views)
        {
            view->Draw(deltaTime);
        }

        if (!_activeSceneView && !_views.empty())
            _activeSceneView = _views[0].get();

        ImGui::Begin("Hierarchy");
        if (_activeSceneView)
        {
            _activeSceneView->OnRenderHierarchy();
        }
        else
        {
            ImGui::TextDisabled("No active scene.");
        }
        ImGui::End();

        ImGui::Begin("Inspector");
        if (_activeSceneView)
        {
            _activeSceneView->OnRenderInspector();
        }
        else
        {
            ImGui::TextDisabled("No active scene.");
        }
        ImGui::End();
    }

    bool EditorLayer::_OnOpenProjectSettings(const OpenProjectSettingsEvent& e)
    {
        if (!_projectSettingsWindow->IsOpen())
        {
            _projectSettingsNeedsDocking = true;
        }

        _projectSettingsWindow->Open();
        return true;
    }

    void EditorLayer::_SetupScriptingWatcher()
    {
        const auto& config = _projectInfo.GetConfig();
        if (config.scriptingModule.empty())
        {
            FT_INFO("No scripting module defined in project. Hot-reload watcher disabled.");
            _scriptingWatcher.Stop();
            return;
        }

        std::filesystem::path projectDir = _projectInfo.GetProjectDir();
        std::filesystem::path dllPath = projectDir / config.scriptingModule;

        _scriptingWatcher.SetPathToWatch(dllPath);
        _scriptingWatcher.Start();
    }

    bool EditorLayer::_OnNewScene(const NewSceneEvent& e)
    {
        return true;
    }
} // namespace Editor