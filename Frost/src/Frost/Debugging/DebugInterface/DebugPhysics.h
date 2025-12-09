#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Debugging/DebugInterface/DebugPanel.h"

namespace Frost
{
    namespace Debug
    {
        struct FROST_API PhysicsConfig
        {
            static bool& IsDisplayEnabled();
        };
    } // namespace Debug

    class FROST_API DebugPhysics : public DebugPanel
    {
    public:
        DebugPhysics() = default;
        virtual ~DebugPhysics() = default;
        virtual void OnImGuiRender(float deltaTime) override;
        virtual const char* GetName() const override { return "Physics"; }
    };
} // namespace Frost