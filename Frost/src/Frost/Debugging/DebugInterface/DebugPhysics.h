#pragma once

#include "Frost/Debugging/DebugInterface/DebugPanel.h"

namespace Frost
{
    namespace Debug
    {
        struct PhysicsConfig
        {
            static bool display;
        };
    } // namespace Debug

    class DebugPhysics : public DebugPanel
    {
    public:
        DebugPhysics() = default;
        virtual ~DebugPhysics() = default;
        virtual void OnImGuiRender(float deltaTime) override;
        virtual const char* GetName() const override { return "Physics"; }
    };
} // namespace Frost