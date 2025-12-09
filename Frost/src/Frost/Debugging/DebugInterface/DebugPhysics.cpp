#include "Frost/Debugging/DebugInterface/DebugPhysics.h"

#include <imgui.h>

namespace Frost
{
    namespace Debug
    {
        bool& PhysicsConfig::IsDisplayEnabled()
        {
            static bool s_Display = false;
            return s_Display;
        }
    } // namespace Debug

    void DebugPhysics::OnImGuiRender(float deltaTime)
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            ImGui::PushID("PhysicsDebugHeader");
            ImGui::Checkbox("Display", &Debug::PhysicsConfig::IsDisplayEnabled());
            ImGui::PopID();
        }
    }
} // namespace Frost