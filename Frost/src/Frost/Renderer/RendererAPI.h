#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Renderer/Renderer.h"
#include "Frost/Utils/NoCopy.h"

namespace Frost
{
    class FROST_API RendererAPI : NoCopy
    {
    public:
        static Renderer* GetRenderer();
        static void SetRenderer(Renderer* renderer);

        static void BeginFrame();
        static void EndFrame();

    private:
        static Renderer* _renderer;
    };
} // namespace Frost