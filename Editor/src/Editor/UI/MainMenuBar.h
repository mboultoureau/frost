#pragma once

#include "Editor/UI/EditorPanel.h"

namespace Editor
{
	class MainMenuBar : public EditorPanel
	{
	public:
		MainMenuBar() = default;
		~MainMenuBar() = default;

		void Draw() override;

	private:
		void _RenderFileMenu();
		void _RenderEditMenu();

		void _RenderProjectName();
	};
}