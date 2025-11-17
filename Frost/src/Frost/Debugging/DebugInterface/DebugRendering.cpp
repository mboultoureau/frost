#include "Frost/Debugging/DebugInterface/DebugRendering.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/RendererDX11.h"

#include <imgui.h>

namespace Frost
{
	bool Debug::RendererConfig::wireframeMode = false;
	bool Debug::RendererConfig::display = true;

	void DebugRendering::OnImGuiRender(float deltaTime)
	{
#ifdef FT_PLATFORM_WINDOWS
		if (ImGui::CollapsingHeader("Rendering"))
		{
			ImGui::Checkbox("Display", &Debug::RendererConfig::display);
			ImGui::Checkbox("Wireframe Mode", &Debug::RendererConfig::wireframeMode);
		}
#endif
	}
}
