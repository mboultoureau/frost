#pragma once

#include "Frost/Core/Core.h"

#include <imgui.h>

namespace Frost
{
    FROST_API void SyncImGuiContext(ImGuiContext* ctx,
                                    ImGuiMemAllocFunc allocFunc,
                                    ImGuiMemFreeFunc freeFunc,
                                    void* userData);
} // namespace Frost