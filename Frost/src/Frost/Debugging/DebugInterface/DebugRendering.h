#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"

namespace Frost
{
    namespace Debug
    {
        struct RendererConfig
        {
            static bool wireframeMode;
            static bool display;
        };
    } // namespace Debug

    class DebugRendering : public DebugPanel
    {
    public:
        DebugRendering() = default;
        virtual ~DebugRendering() override = default;
        virtual void OnImGuiRender(float deltaTime) override;
        virtual const char* GetName() const override { return "Rendering"; }
    };
} // namespace Frost
