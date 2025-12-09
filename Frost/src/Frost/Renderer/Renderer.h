#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Core/Core.h"
#include "Frost/Core/Window.h"
#include "Frost/Event/EventManager.h"
#include "Frost/Event/Events/Window/WindowResizeEvent.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/CommandList.h"
#include "Frost/Renderer/Pipeline.h"

#include <memory>

namespace Frost
{
    class FROST_API Renderer
    {
    public:
        Renderer();
        virtual ~Renderer();

        virtual void OnWindowResize(WindowResizeEvent& resizeEvent) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void RestoreBackBufferRenderTarget() = 0;

        virtual std::shared_ptr<CommandList> GetNewCommandList() = 0;
        virtual Texture* GetBackBuffer() = 0;
        virtual Texture* GetDepthBuffer() = 0;

        virtual std::shared_ptr<Buffer> CreateBuffer(const BufferConfig& config, const void* initialData = nullptr) = 0;

        void RegisterPipeline(Pipeline* pipeline);
        void UnregisterPipeline(Pipeline* pipeline);

    private:
        EventHandlerId _resizeEventHandlerId;
        std::vector<Pipeline*> _pipelines;
    };
} // namespace Frost