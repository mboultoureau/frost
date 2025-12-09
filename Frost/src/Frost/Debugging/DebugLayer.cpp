#include "Frost/Debugging/DebugLayer.h"

#include "Frost/Core/Application.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Event/Events/Input/KeyPressedEvent.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include "Frost/Debugging/DebugInterface/DebugInput.h"
#include "Frost/Debugging/DebugInterface/DebugPerformance.h"
#include "Frost/Debugging/DebugInterface/DebugPhysics.h"
#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Debugging/DebugInterface/DebugScene.h"
#include "Frost/Debugging/DebugInterface/DebugWindow.h"
#include "Frost/Input/Input.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace Frost
{

    DebugLayer::DebugLayer() : Layer(GetStaticName()) {}

    void DebugLayer::OnAttach()
    {
        if (ImGui::GetCurrentContext() == nullptr)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        }

        // Setup Platform/Renderer backends
#ifdef FT_PLATFORM_WINDOWS
        WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());

        ImGui_ImplWin32_Init(window->GetWindowHandle());
        ImGui_ImplDX11_Init(renderer->GetDevice(), renderer->GetDeviceContext());
#else
#error "Platform not supported!"
#endif

        _debugPanels.push_back(std::make_unique<DebugInput>());
        _debugPanels.push_back(std::make_unique<DebugPerformance>());
        _debugPanels.push_back(std::make_unique<DebugPhysics>());
        _debugPanels.push_back(std::make_unique<DebugRendering>());
        _debugPanels.push_back(std::make_unique<DebugScene>());
        _debugPanels.push_back(std::make_unique<DebugWindow>());
    }

    void DebugLayer::OnDetach()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        _debugPanels.clear();
        //_logTimer.Pause();
    }

    void DebugLayer::OnUpdate(float deltaTime)
    {
        //        if (ImGui::IsKeyPressed(ImGuiKey_F1))
        //        {
        //            _displayDebug = !_displayDebug;
        //        }

        _displayDebug = true;

        if (!_displayDebug)
            return;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (!ImGui::Begin("Debug", nullptr, 0))
        {
            ImGui::End();
            return;
        }

        for (auto& panel : _debugPanels)
        {
            panel->OnUpdate(deltaTime);
        }

        for (auto& panel : _debugPanels)
        {
            panel->OnImGuiRender(deltaTime);
        }

        ImGui::End();
    }

    void DebugLayer::OnLateUpdate(float deltaTime)
    {
        if (!_displayDebug)
            return;

        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        for (auto& panel : _debugPanels)
        {
            panel->OnLateUpdate(deltaTime);
        }
    }

    void DebugLayer::AddScene(Scene* scene)
    {
        // Ignore if scene is null
        if (!scene)
        {
            return;
        }

        for (auto& panel : _debugPanels)
        {
            if (auto debugScenePanel = dynamic_cast<DebugScene*>(panel.get()))
            {
                debugScenePanel->AddScene(scene);
                return;
            }
        }
    }

    void DebugLayer::RemoveScene(Scene* scene)
    {
        for (auto& panel : _debugPanels)
        {
            if (auto debugScenePanel = dynamic_cast<DebugScene*>(panel.get()))
            {
                debugScenePanel->RemoveScene(scene);
                return;
            }
        }
    }

    void DebugLayer::OnFixedUpdate(float fixedDeltaTime)
    {
        for (auto& panel : _debugPanels)
        {
            panel->OnFixedUpdate(fixedDeltaTime);
        }
    }
} // namespace Frost