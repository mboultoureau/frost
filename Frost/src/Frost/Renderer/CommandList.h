#pragma once

#include "Frost/Core/Core.h"
#include "Frost/Renderer/GraphicsTypes.h"
#include "Frost/Renderer/Viewport.h"

#include <cstdint>

namespace Frost
{
    class Texture;
    class Shader;
    class InputLayout;
    class Buffer;
    class Sampler;

    enum class BlendMode
    {
        None,
        Alpha
    };

    enum class DepthMode
    {
        None,
        ReadWrite,
        ReadOnly
    };

    enum class RasterizerMode
    {
        Solid,
        Wireframe,
        SolidCullNone
    };

    class CommandList
    {
    public:
        virtual ~CommandList() = default;

        virtual void BeginRecording() = 0;
        virtual void EndRecording() = 0;
        virtual void Execute() = 0;

        virtual void SetRasterizerState(RasterizerMode mode) = 0;
        virtual void SetRenderTargets(uint32_t count, Texture** renderTargets, Texture* depthStencil = nullptr) = 0;
        virtual void ClearRenderTarget(Texture* renderTarget, const float color[4]) = 0;
        virtual void ClearRenderTarget(Texture* renderTarget, const float color[4], Viewport viewport) = 0;
        virtual void ClearDepthStencil(Texture* depthStencil, bool clearDepth, float depthValue, bool clearStencil, uint8_t stencilValue) = 0;

        virtual void SetScissorRect(int x, int y, int width, int height) = 0;
        virtual void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) = 0;

        virtual void SetBlendState(BlendMode mode) = 0;
        virtual void SetDepthStencilState(DepthMode mode) = 0;

        virtual void SetShader(const Shader* shader) = 0;
        virtual void SetInputLayout(const InputLayout* layout) = 0;
        virtual void SetVertexBuffer(const Buffer* buffer, uint32_t stride, uint32_t offset) = 0;
        virtual void SetIndexBuffer(const Buffer* buffer, uint32_t offset) = 0;
        virtual void SetConstantBuffer(const Buffer* buffer, uint32_t slot) = 0;
        virtual void SetTexture(const Texture* texture, uint32_t slot) = 0;
        virtual void SetSampler(const Sampler* sampler, uint32_t slot) = 0;

        virtual void SetPrimitiveTopology(PrimitiveTopology topology) = 0;
        virtual void CopyResource(Texture* destination, Texture* source) = 0;

        virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation) = 0;

        virtual void* GetNativeRenderContext() = 0;
    };
}