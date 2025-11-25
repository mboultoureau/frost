#include "Editor/EditorLayer.h"
#include "Editor/UI/EditorTheme.h"
#include "Frost/Core/Windows/WindowWin.h"
#include "Frost/Core/Application.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/RendererDX11.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <imgui_internal.h>

using namespace Frost;

namespace Editor
{
	EditorLayer::EditorLayer() : Frost::Layer(GetStaticName())
	{
	}

	void EditorLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

		EditorTheme::Apply();

#ifdef FT_PLATFORM_WINDOWS
		WindowWin* window = static_cast<WindowWin*>(Application::GetWindow());
		RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());

		ImGui_ImplWin32_Init(window->GetWindowHandle());
		ImGui_ImplDX11_Init(renderer->GetDevice(), renderer->GetDeviceContext());
#else
		#error "Platform not supported!"
#endif

		// Initialize Editor UI components
		_mainMenuBar = std::make_unique<MainMenuBar>();
		_contentBrowser = std::make_unique<ContentBrowser>();
		_statusBar = std::make_unique<StatusBar>();

		_views.push_back(std::make_unique<SceneView>("Scene Level 1"));
		_views.push_back(std::make_unique<SceneView>("Player Prefab"));
	}

	void EditorLayer::OnDetach()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorLayer::OnUpdate(float deltaTime)
	{
	}

	void EditorLayer::OnLateUpdate(float deltaTime)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		_RenderUI();
		
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void EditorLayer::OnFixedUpdate(float fixedDeltaTime)
	{
	}

	void EditorLayer::_RenderUI()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGuiID dockspaceID = ImGui::GetID("DockSpace");

		_statusBar->Draw();

		ImGui::DockSpaceOverViewport(dockspaceID, viewport);

		static bool firstTime = true;
		if (firstTime)
		{
			firstTime = false;

			ImGui::DockBuilderRemoveNode(dockspaceID);
			ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->WorkSize);

			ImGuiID dock_main_id = dockspaceID;
			ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);

			ImGui::DockBuilderDockWindow("Content Browser", dock_bottom_id);

			ImGui::DockBuilderDockWindow("Scene Level 1", dock_main_id);
			ImGui::DockBuilderDockWindow("Player Prefab", dock_main_id);

			ImGui::DockBuilderFinish(dockspaceID);
		}

		_mainMenuBar->Draw();
		_contentBrowser->Draw();

		for (auto& view : _views)
		{
			view->Draw();
		}
	}
}