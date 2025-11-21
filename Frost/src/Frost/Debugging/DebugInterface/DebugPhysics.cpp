#include "Frost/Debugging/DebugInterface/DebugPhysics.h"

#include <imgui.h>

namespace Frost
{
	bool Debug::PhysicsConfig::display = false;

	void DebugPhysics::OnImGuiRender(float deltaTime)
	{
		if (ImGui::CollapsingHeader("Physics"))
		{
			ImGui::PushID("PhysicsDebugHeader");
			ImGui::Checkbox("Display", &Debug::PhysicsConfig::display);
			ImGui::PopID();
		}
	}
}