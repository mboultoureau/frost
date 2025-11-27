#pragma once

#include "Frost/Renderer/CommandList.h"
#include "Frost/Renderer/Viewport.h"

#include <d3d11_1.h>
#include <wrl/client.h>

namespace Frost
{
    class CommandListDX11 : public CommandList
    {
    public:
        CommandListDX11();
        virtual ~CommandListDX11() override = default;

        void BeginRecording() override;
        void EndRecording() override;
        void Execute() override;

        void SetRasterizerState(RasterizerMode mode) override;
        void SetRenderTargets(uint32_t count, Texture** renderTargets, Texture* depthStencil) override;
        void ClearRenderTarget(Texture* renderTarget, const float color[4]) override;
        void ClearRenderTarget(Texture* renderTarget, const float color[4], const Viewport viewport) override;
        void ClearDepthStencil(Texture* depthStencil,
                               bool clearDepth,
                               float depthValue,
                               bool clearStencil,
                               uint8_t stencilValue) override;

        void SetScissorRect(int x, int y, int width, int height) override;
        void SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth) override;

        void SetBlendState(BlendMode mode) override;
        void SetDepthStencilState(DepthMode mode) override;

        void SetShader(const Shader* shader) override;
        void UnbindShader(ShaderType type) override;
        void SetInputLayout(const InputLayout* layout) override;
        void SetVertexBuffer(const Buffer* buffer, uint32_t stride, uint32_t offset) override;
        void SetIndexBuffer(const Buffer* buffer, uint32_t offset) override;
        void SetConstantBuffer(const Buffer* buffer, uint32_t slot) override;
        void SetTexture(const Texture* texture, uint32_t slot) override;
        void SetSampler(const Sampler* sampler, uint32_t slot) override;

        void SetPrimitiveTopology(PrimitiveTopology topology) override;

        void CopyResource(Texture* destination, Texture* source) override;

        void Draw(uint32_t vertexCount, uint32_t startVertexLocation) override;
        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation) override;

        virtual void* GetNativeRenderContext() override { return _context.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D11DeviceContext1> _context;
        Microsoft::WRL::ComPtr<ID3D11CommandList> _commandList;
    };
} // namespace Frost