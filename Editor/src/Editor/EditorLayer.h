#pragma once

#include "Frost.h"

#include "Editor/UI/MainMenuBar.h"
#include "Editor/UI/ContentBrowser.h"
#include "Editor/UI/StatusBar.h"
#include "Editor/UI/SceneView.h"

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

	private:
		void _RenderUI();

	private:
		std::unique_ptr<MainMenuBar> _mainMenuBar;
		std::unique_ptr<ContentBrowser> _contentBrowser;
		std::unique_ptr<StatusBar> _statusBar;
		std::vector<std::unique_ptr<SceneView>> _views;
	};
}


