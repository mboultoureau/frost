#include "SceneView.h"

#include <imgui_internal.h>

namespace Editor
{
	SceneView::SceneView(const std::string& title) : _title(title)
	{
	}

	void SceneView::Draw()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		bool open = true;

		if (ImGui::Begin(_title.c_str(), &open))
		{
			_dockSpaceID = ImGui::GetID(("DockSpace_" + _title).c_str());
			ImGui::DockSpace(_dockSpaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

			if (!_isLayoutInitialized && ImGui::GetContentRegionAvail().x > 100.0f)
			{
				_isLayoutInitialized = true;

				ImGui::DockBuilderRemoveNode(_dockSpaceID);
				ImGui::DockBuilderAddNode(_dockSpaceID, ImGuiDockNodeFlags_DockSpace);
				ImGui::DockBuilderSetNodeSize(_dockSpaceID, ImGui::GetWindowSize());

				ImGuiID dock_main_id = _dockSpaceID;

				ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.30f, nullptr, &dock_main_id);
				ImGuiID dock_right_bottom_id = ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.50f, nullptr, &dock_right_id);

				ImGui::DockBuilderDockWindow(_GetWindowName("Viewport").c_str(), dock_main_id);
				ImGui::DockBuilderDockWindow(_GetWindowName("Hierarchy").c_str(), dock_right_id);
				ImGui::DockBuilderDockWindow(_GetWindowName("Inspector").c_str(), dock_right_bottom_id);

				ImGui::DockBuilderFinish(_dockSpaceID);
			}

			_DrawViewport();
			_DrawHierarchy();
			_DrawInspector();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneView::_DrawViewport()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin(_GetWindowName("Viewport").c_str());

		_DrawToolbar();

		// ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		// ImGui::Image((void*)textureID, viewportSize);
		ImGui::Text(" [ RENDER TARGET: %s ]", _title.c_str());

		// Drag & Drop
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const char* file = (const char*)payload->Data;
				printf("Dropped %s into %s\n", file, _title.c_str());
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void SceneView::_DrawToolbar()
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); // Petit padding
		if (ImGui::Button("Move")) {} ImGui::SameLine();
		if (ImGui::Button("Rotate")) {} ImGui::SameLine();
		if (ImGui::Button("Scale")) {}

		ImGui::Separator();
	}

	void SceneView::_DrawHierarchy()
	{
		ImGui::Begin(_GetWindowName("Hierarchy").c_str());

		static int selected = 0;
		for (int i = 0; i < 5; i++) {
			std::string name = "Entity_" + std::to_string(i);
			if (ImGui::Selectable(name.c_str(), selected == i)) {
				selected = i;
			}
		}
		ImGui::End();
	}

	void SceneView::_DrawInspector()
	{
		ImGui::Begin(_GetWindowName("Inspector").c_str());

		ImGui::TextDisabled("Properties for %s", _title.c_str());
		ImGui::Separator();

		ImGui::Text("Transform");
		static float pos[3] = { 0.0f, 0.0f, 0.0f };
		ImGui::DragFloat3("Position", pos, 0.1f);
		ImGui::DragFloat3("Rotation", pos, 0.1f);
		ImGui::DragFloat3("Scale", pos, 0.1f);

		ImGui::Separator();
		ImGui::Text("Components");
		ImGui::Button("Add Component", ImVec2(-1, 0));

		ImGui::End();
	}

	std::string SceneView::_GetWindowName(const std::string& baseName)
	{
		return baseName + "###" + baseName + "_" + _title;
	}
}