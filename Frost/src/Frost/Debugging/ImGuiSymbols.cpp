#include "Frost/Debugging/ImGuiSymbols.h"

namespace Frost
{
    void SyncImGuiContext(ImGuiContext* ctx, ImGuiMemAllocFunc allocFunc, ImGuiMemFreeFunc freeFunc, void* userData)
    {
        ImGui::SetCurrentContext(ctx);
        ImGui::SetAllocatorFunctions(allocFunc, freeFunc, userData);
    }
} // namespace Frost