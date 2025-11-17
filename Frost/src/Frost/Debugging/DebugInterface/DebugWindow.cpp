#include "Frost/Debugging/DebugInterface/DebugWindow.h"
#include "Frost/Core/Application.h"

#include <imgui.h>

namespace Frost
{
	void DebugWindow::OnImGuiRender(float deltaTime)
	{
		if (ImGui::CollapsingHeader("Window"))
		{
			ImGui::Text("Width: %u", Application::GetWindow()->GetWidth());
			ImGui::Text("Height: %u", Application::GetWindow()->GetHeight());
			
			bool isFullscreen = Application::GetWindow()->IsFullscreen();
			if (ImGui::Checkbox("Fullscreen", &isFullscreen))
			{
				Application::GetWindow()->SetFullscreen(isFullscreen);
			}
		}
	}
}

