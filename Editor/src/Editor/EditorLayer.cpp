#include "Editor/EditorLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/UI/EditorTheme.h"
#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Scene/Serializers/EngineComponentSerializer.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <imgui_internal.h>

using namespace Frost;

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
        ImGuiIO& io = ImGui::GetIO();
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

        // Register engine component serializers
        EngineComponentSerializer::RegisterEngineComponents();

        auto& app = EditorApp::Get();
        _projectInfo = app.GetProjectInfo();

        // Initialize Editor UI components
        _mainMenuBar = std::make_unique<MainMenuBar>(_projectInfo);
        _contentBrowser = std::make_unique<ContentBrowser>(_projectInfo);
        _statusBar = std::make_unique<StatusBar>();
    }

    void EditorLayer::OnDetach()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorLayer::OnUpdate(float deltaTime) {}

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

    void EditorLayer::OnFixedUpdate(float fixedDeltaTime) {}

    EditorLayer& EditorLayer::Get()
    {
        FT_ENGINE_ASSERT(_instance, "EditorLayer instance is null!");
        return *_instance;
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

        if (_dockMainID != 0)
        {
            ImGui::DockBuilderDockWindow(newView->GetTitle().c_str(), _dockMainID);
        }

        _activeSceneView = newView.get();
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
        if (_dockMainID != 0)
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

        static bool firstTime = true;
        if (firstTime)
        {
            firstTime = false;

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
} // namespace Editor