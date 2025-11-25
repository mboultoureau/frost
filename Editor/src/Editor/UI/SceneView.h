#pragma once

#include "Editor/UI/EditorPanel.h"

#include <imgui.h>
#include <string>

namespace Editor
{
	class SceneView : public EditorPanel
	{
	public:
		SceneView(const std::string& title);

		void Draw() override;

	private:
		void _DrawViewport();
		void _DrawToolbar();
		void _DrawHierarchy();
		void _DrawInspector();

		std::string _GetWindowName(const std::string& baseName);

	private:
		std::string _title{ "Scene View" };
		bool _isLayoutInitialized = false;
		ImGuiID _dockSpaceID = 0;
	};
}


