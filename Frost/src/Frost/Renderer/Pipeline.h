#pragma once

#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Renderer/GPUResource.h"

#include <cstdint>

namespace Frost
{
    class Pipeline : public GPUResource
    {
    public:
        Pipeline() = default;
        virtual ~Pipeline() = default;

        virtual void Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void OnWindowResize(WindowResizeEvent& resizeEvent) {};
    };
} // namespace Frost