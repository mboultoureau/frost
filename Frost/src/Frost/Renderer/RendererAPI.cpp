#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
    Renderer* RendererAPI::_renderer = nullptr;

    void RendererAPI::SetRenderer(Renderer* renderer)
    {
        _renderer = renderer;
    }

    void RendererAPI::BeginFrame()
    {
        FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::BeginFrame called but no renderer is set!");
        _renderer->BeginFrame();
    }

    void RendererAPI::EndFrame()
    {
        FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::EndFrame called but no renderer is set!");
        _renderer->EndFrame();
    }

    Renderer* RendererAPI::GetRenderer()
    {
        FT_ENGINE_ASSERT(_renderer != nullptr, "RendererAPI::Get called but no renderer is set!");
        return _renderer;
    }
} // namespace Frost