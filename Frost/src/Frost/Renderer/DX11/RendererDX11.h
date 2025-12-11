#pragma once

#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/Renderer.h"

#include <d3d11_1.h>
#include <wrl.h>

namespace Frost
{
    class RendererDX11 : public Renderer
    {
    public:
        RendererDX11();
        virtual ~RendererDX11() override;

        virtual void OnWindowResize(WindowResizeEvent& resizeEvent) override;
        virtual void BeginFrame() override;
        virtual void EndFrame() override;
        virtual void RestoreBackBufferRenderTarget() override;

        virtual std::shared_ptr<CommandList> GetNewCommandList() override;
        virtual Texture* GetBackBuffer() override { return _backBufferTexture.get(); }
        virtual Texture* GetDepthBuffer() override { return _depthBufferTexture.get(); }
        std::shared_ptr<Buffer> CreateBuffer(const BufferConfig& config, const void* initialData) override;

        ID3D11Device1* GetDevice() const { return _device.Get(); }
        ID3D11DeviceContext* GetDeviceContext() const { return _immediateContext.Get(); }
        ID3D11DepthStencilView* GetDepthStencilView() const { return _depthStencilView.Get(); }
        ID3D11RasterizerState* GetSolidRasterizerState() const { return _solidRasterizerState.Get(); }
        ID3D11RasterizerState* GetWireframeRasterizerState() const { return _wireframeRasterizerState.Get(); }
        ID3D11RasterizerState* GetCullNoneRasterizerState() const { return _cullNoneRasterizerState.Get(); }
        ID3D11RasterizerState* GetCullBackRasterizerState() const { return _cullBackRasterizerState.Get(); }
        ID3D11BlendState* GetBlendState(BlendMode mode) const;
        ID3D11DepthStencilState* GetDepthStencilState(DepthMode mode) const;

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory> _dxgiFactory;
        Microsoft::WRL::ComPtr<ID3D11Device1> _device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> _immediateContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferRTV;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> _depthTexture;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _solidRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _wireframeRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _cullNoneRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _cullBackRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> _cullFrontRasterizerState;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depthStateReadWrite;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depthStateReadOnly;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> _depthStateNone;
        Microsoft::WRL::ComPtr<ID3D11BlendState> _blendStateAlpha;
        std::unique_ptr<TextureDX11> _backBufferTexture;
        std::unique_ptr<TextureDX11> _depthBufferTexture;

    private:
        void _CreateDevice();
        void _CreateRenderTargets();
        void _CreateDepthBuffer();
        void _CreateDepthStencilStates();
        void _CreateRasterizerStates();
        void _CreateBlendStates();
    };
} // namespace Frost