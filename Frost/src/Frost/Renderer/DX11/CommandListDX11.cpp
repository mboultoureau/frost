#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/DX11/BufferDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include "Frost/Debugging/Assert.h"
#include <vector>

namespace Frost
{
    CommandListDX11::CommandListDX11()
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device1* device = renderer->GetDevice();
        HRESULT hr = device->CreateDeferredContext1(0, &_context);
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create deferred device context.");
    }

    void CommandListDX11::BeginRecording()
    {
        _commandList.Reset();
    }

    void CommandListDX11::EndRecording()
    {
        HRESULT hr = _context->FinishCommandList(FALSE, &_commandList);
        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to finish command list recording.");
    }

    void CommandListDX11::Execute()
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11DeviceContext* immediateContext = renderer->GetDeviceContext();
        immediateContext->ExecuteCommandList(_commandList.Get(), FALSE);
    }

    void CommandListDX11::SetRasterizerState(RasterizerMode mode)
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        switch (mode)
        {
            case RasterizerMode::Solid:
                _context->RSSetState(renderer->GetSolidRasterizerState());
                break;
            case RasterizerMode::Wireframe:
                _context->RSSetState(renderer->GetWireframeRasterizerState());
                break;
            case RasterizerMode::SolidCullNone:
                _context->RSSetState(renderer->GetCullNoneRasterizerState());
                break;
            case RasterizerMode::SolidCullBack:
                _context->RSSetState(renderer->GetCullBackRasterizerState());
                break;
            case RasterizerMode::SolidCullFront:
                _context->RSSetState(renderer->GetCullBackRasterizerState());
                break;
        }
    }

    void CommandListDX11::SetRenderTargets(uint32_t count, Texture** renderTargets, Texture* depthStencil)
    {
        std::vector<ID3D11RenderTargetView*> rtvs(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            rtvs[i] = static_cast<TextureDX11*>(renderTargets[i])->GetRTV();
        }

        ID3D11DepthStencilView* dsv = depthStencil ? static_cast<TextureDX11*>(depthStencil)->GetDSV() : nullptr;

        _context->OMSetRenderTargets(count, rtvs.data(), dsv);
    }

    void CommandListDX11::ClearRenderTarget(Texture* renderTarget, const float color[4])
    {
        FT_ENGINE_ASSERT(renderTarget, "Render target cannot be null.");
        ID3D11RenderTargetView* rtv = static_cast<TextureDX11*>(renderTarget)->GetRTV();
        _context->ClearRenderTargetView(rtv, color);
    }

    void CommandListDX11::ClearRenderTarget(Texture* renderTarget, const float color[4], const Viewport viewport)
    {
        FT_ENGINE_ASSERT(renderTarget, "Render target cannot be null.");
        auto* dxTexture = static_cast<TextureDX11*>(renderTarget);
        ID3D11RenderTargetView* rtv = dxTexture->GetRTV();

        if (!rtv)
        {
            FT_ENGINE_WARN("CommandListDX11: Tentative de Clear sur un RTV null/invalide.");
            return;
        }

        D3D11_RECT rect;
        rect.left =
            static_cast<LONG>(viewport.x * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetWidth()));
        rect.top =
            static_cast<LONG>(viewport.y * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetHeight()));
        rect.right = static_cast<LONG>((viewport.x + viewport.width) *
                                       static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetWidth()));
        rect.bottom = static_cast<LONG>((viewport.y + viewport.height) *
                                        static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetHeight()));

        _context->ClearView(rtv, color, &rect, 1);
    }

    void CommandListDX11::ClearDepthStencil(Texture* depthStencil,
                                            bool clearDepth,
                                            float depthValue,
                                            bool clearStencil,
                                            uint8_t stencilValue)
    {
        FT_ENGINE_ASSERT(depthStencil, "Depth stencil cannot be null.");
        ID3D11DepthStencilView* dsv = static_cast<TextureDX11*>(depthStencil)->GetDSV();

        UINT clearFlags = 0;
        if (clearDepth)
            clearFlags |= D3D11_CLEAR_DEPTH;
        if (clearStencil)
            clearFlags |= D3D11_CLEAR_STENCIL;

        _context->ClearDepthStencilView(dsv, clearFlags, depthValue, stencilValue);
    }

    void CommandListDX11::SetScissorRect(int x, int y, int width, int height)
    {
        D3D11_RECT scissorRect;
        scissorRect.left = x;
        scissorRect.top = y;
        scissorRect.right = x + width;
        scissorRect.bottom = y + height;
        _context->RSSetScissorRects(1, &scissorRect);
    }

    void CommandListDX11::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = x;
        viewport.TopLeftY = y;
        viewport.Width = width;
        viewport.Height = height;
        viewport.MinDepth = minDepth;
        viewport.MaxDepth = maxDepth;
        _context->RSSetViewports(1, &viewport);
    }

    void CommandListDX11::SetBlendState(BlendMode mode)
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11BlendState* state = renderer->GetBlendState(mode);
        _context->OMSetBlendState(state, nullptr, 0xFFFFFFFF);
    }

    void CommandListDX11::SetDepthStencilState(DepthMode mode)
    {
        RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11DepthStencilState* state = renderer->GetDepthStencilState(mode);
        _context->OMSetDepthStencilState(state, 1);
    }

    void CommandListDX11::SetShader(const Shader* shader)
    {
        FT_ENGINE_ASSERT(shader, "Shader cannot be null.");
        const ShaderDX11* shaderDX11 = static_cast<const ShaderDX11*>(shader);

        switch (shader->GetType())
        {
            case ShaderType::Vertex:
                _context->VSSetShader(static_cast<ID3D11VertexShader*>(shaderDX11->GetShaderObject()), nullptr, 0);
                break;
            case ShaderType::Pixel:
                _context->PSSetShader(static_cast<ID3D11PixelShader*>(shaderDX11->GetShaderObject()), nullptr, 0);
                break;
            case ShaderType::Geometry:
                _context->GSSetShader(static_cast<ID3D11GeometryShader*>(shaderDX11->GetShaderObject()), nullptr, 0);
                break;
            case ShaderType::Hull:
                _context->HSSetShader(static_cast<ID3D11HullShader*>(shaderDX11->GetShaderObject()), nullptr, 0);
                break;
            case ShaderType::Domain:
                _context->DSSetShader(static_cast<ID3D11DomainShader*>(shaderDX11->GetShaderObject()), nullptr, 0);
                break;
        }
    }

    void CommandListDX11::UnbindShader(ShaderType type)
    {
        switch (type)
        {
            case ShaderType::Vertex:
                _context->VSSetShader(nullptr, nullptr, 0);
                break;
            case ShaderType::Pixel:
                _context->PSSetShader(nullptr, nullptr, 0);
                break;
            case ShaderType::Geometry:
                _context->GSSetShader(nullptr, nullptr, 0);
                break;
            case ShaderType::Hull:
                _context->HSSetShader(nullptr, nullptr, 0);
                break;
            case ShaderType::Domain:
                _context->DSSetShader(nullptr, nullptr, 0);
                break;
            case ShaderType::Compute:
                _context->CSSetShader(nullptr, nullptr, 0);
                break;
        }
    }

    void CommandListDX11::SetInputLayout(const InputLayout* inputLayout)
    {
        if (inputLayout)
        {
            const auto* layoutDX11 = static_cast<const InputLayoutDX11*>(inputLayout);
            _context->IASetInputLayout(layoutDX11->GetLayout());
        }
        else
        {
            _context->IASetInputLayout(nullptr);
        }
    }

    void CommandListDX11::SetVertexBuffer(const Buffer* vertexBuffer, uint32_t stride, uint32_t offset)
    {
        FT_ENGINE_ASSERT(vertexBuffer, "Vertex buffer cannot be null.");
        const auto* bufferDX11 = static_cast<const BufferDX11*>(vertexBuffer);
        ID3D11Buffer* buffer = bufferDX11->GetD3D11Buffer();

        ID3D11Buffer* const pBuffers[] = { buffer };
        const UINT pStrides[] = { stride };
        const UINT pOffsets[] = { offset };
        _context->IASetVertexBuffers(0, 1, pBuffers, pStrides, pOffsets);
    }

    void CommandListDX11::SetIndexBuffer(const Buffer* indexBuffer, uint32_t offset)
    {
        FT_ENGINE_ASSERT(indexBuffer, "Index buffer cannot be null.");
        const auto* bufferDX11 = static_cast<const BufferDX11*>(indexBuffer);

        uint32_t stride = bufferDX11->GetConfig().stride;
        DXGI_FORMAT format = (stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

        _context->IASetIndexBuffer(bufferDX11->GetD3D11Buffer(), format, offset);
    }

    void CommandListDX11::SetConstantBuffer(const Buffer* buffer, uint32_t slot)
    {
        FT_ENGINE_ASSERT(buffer, "Constant buffer cannot be null.");

        ID3D11Buffer* d3dBuffer = static_cast<const BufferDX11*>(buffer)->GetD3D11Buffer();

        ID3D11Buffer* const pBuffers[] = { d3dBuffer };

        _context->VSSetConstantBuffers(slot, 1, pBuffers);
        _context->PSSetConstantBuffers(slot, 1, pBuffers);
        _context->GSSetConstantBuffers(slot, 1, pBuffers);
        _context->HSSetConstantBuffers(slot, 1, pBuffers);
        _context->DSSetConstantBuffers(slot, 1, pBuffers);
    }

    void CommandListDX11::SetTexture(const Texture* texture, uint32_t slot)
    {
        ID3D11ShaderResourceView* srv = nullptr;
        if (texture)
        {
            srv = static_cast<const TextureDX11*>(texture)->GetSRV();
        }
        _context->VSSetShaderResources(slot, 1, &srv);
        _context->PSSetShaderResources(slot, 1, &srv);
    }

    void CommandListDX11::SetSampler(const Sampler* sampler, uint32_t slot)
    {
        FT_ENGINE_ASSERT(sampler, "Sampler cannot be null.");
        ID3D11SamplerState* ss = static_cast<const SamplerDX11*>(sampler)->GetSamplerState();
        _context->VSSetSamplers(slot, 1, &ss);
        _context->PSSetSamplers(slot, 1, &ss);
    }

    void CommandListDX11::SetPrimitiveTopology(PrimitiveTopology topology)
    {
        D3D11_PRIMITIVE_TOPOLOGY dxTopology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        switch (topology)
        {
            case PrimitiveTopology::POINTLIST:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
                break;
            case PrimitiveTopology::LINELIST:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
                break;
            case PrimitiveTopology::LINESTRIP:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
                break;
            case PrimitiveTopology::TRIANGLELIST:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
                break;
            case PrimitiveTopology::TRIANGLESTRIP:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
                break;
            case PrimitiveTopology::PATCHLIST_3:
                dxTopology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
                break;
        }

        _context->IASetPrimitiveTopology(dxTopology);
    }

    void CommandListDX11::CopyResource(Texture* destination, Texture* source)
    {
        auto* destDX11 = static_cast<TextureDX11*>(destination);
        auto* srcDX11 = static_cast<TextureDX11*>(source);

        if (!destDX11 || !srcDX11)
        {
            return;
        }

        ID3D11Resource* pDstResource = destDX11->GetDX11Texture();
        ID3D11Resource* pSrcResource = srcDX11->GetDX11Texture();

        if (pDstResource && pSrcResource)
        {
            _context->CopyResource(pDstResource, pSrcResource);
        }
    }

    void CommandListDX11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
    {
        _context->Draw(vertexCount, startVertexLocation);
    }

    void CommandListDX11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation)
    {
        _context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }

    D3D11_STENCIL_OP CommandListDX11::ConvertStencilOp(StencilOp op)
    {
        switch (op)
        {
            case StencilOp::Keep:
                return D3D11_STENCIL_OP_KEEP;
            case StencilOp::Zero:
                return D3D11_STENCIL_OP_ZERO;
            case StencilOp::Replace:
                return D3D11_STENCIL_OP_REPLACE;
            case StencilOp::IncrementSaturate:
                return D3D11_STENCIL_OP_INCR_SAT;
            case StencilOp::DecrementSaturate:
                return D3D11_STENCIL_OP_DECR_SAT;
            case StencilOp::Invert:
                return D3D11_STENCIL_OP_INVERT;
            case StencilOp::Increment:
                return D3D11_STENCIL_OP_INCR;
            case StencilOp::Decrement:
                return D3D11_STENCIL_OP_DECR;
            default:
                return D3D11_STENCIL_OP_KEEP;
        }
    }

    D3D11_COMPARISON_FUNC CommandListDX11::ConvertCompareFunc(CompareFunction func)
    {
        switch (func)
        {
            case CompareFunction::Never:
                return D3D11_COMPARISON_NEVER;
            case CompareFunction::Less:
                return D3D11_COMPARISON_LESS;
            case CompareFunction::Equal:
                return D3D11_COMPARISON_EQUAL;
            case CompareFunction::LessEqual:
                return D3D11_COMPARISON_LESS_EQUAL;
            case CompareFunction::Greater:
                return D3D11_COMPARISON_GREATER;
            case CompareFunction::NotEqual:
                return D3D11_COMPARISON_NOT_EQUAL;
            case CompareFunction::GreaterEqual:
                return D3D11_COMPARISON_GREATER_EQUAL;
            case CompareFunction::Always:
                return D3D11_COMPARISON_ALWAYS;
            default:
                return D3D11_COMPARISON_ALWAYS;
        }
    }

    void CommandListDX11::SetDepthStencilStateCustom(bool depthEnable,
                                                     bool depthWrite,
                                                     CompareFunction depthFunc,
                                                     bool stencilEnable,
                                                     CompareFunction stencilFunc,
                                                     StencilOp stencilFailOp,
                                                     StencilOp depthFailOp,
                                                     StencilOp passOp,
                                                     uint8_t stencilRef,
                                                     uint8_t stencilReadMask,
                                                     uint8_t stencilWriteMask)
    {
        // Creer une cle de hash unique pour cacher l'etat
        size_t key = 0;
        key ^= std::hash<bool>{}(depthEnable) << 0;
        key ^= std::hash<bool>{}(depthWrite) << 1;
        key ^= std::hash<int>{}(static_cast<int>(depthFunc)) << 2;
        key ^= std::hash<bool>{}(stencilEnable) << 3;
        key ^= std::hash<int>{}(static_cast<int>(stencilFunc)) << 4;
        key ^= std::hash<int>{}(static_cast<int>(stencilFailOp)) << 5;
        key ^= std::hash<int>{}(static_cast<int>(depthFailOp)) << 6;
        key ^= std::hash<int>{}(static_cast<int>(passOp)) << 7;
        key ^= std::hash<uint8_t>{}(stencilReadMask) << 8;
        key ^= std::hash<uint8_t>{}(stencilWriteMask) << 9;

        // Chercher dans le cache
        auto it = _depthStencilCache.find(key);
        ID3D11DepthStencilState* state = nullptr;

        if (it != _depthStencilCache.end())
        {
            state = it->second.Get();
        }
        else
        {
            // Creer un nouvel etat
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = depthEnable;
            desc.DepthWriteMask = depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.DepthFunc = ConvertCompareFunc(depthFunc);
            desc.StencilEnable = stencilEnable;
            desc.StencilReadMask = stencilReadMask;
            desc.StencilWriteMask = stencilWriteMask;

            desc.FrontFace.StencilFunc = ConvertCompareFunc(stencilFunc);
            desc.FrontFace.StencilFailOp = ConvertStencilOp(stencilFailOp);
            desc.FrontFace.StencilDepthFailOp = ConvertStencilOp(depthFailOp);
            desc.FrontFace.StencilPassOp = ConvertStencilOp(passOp);

            desc.BackFace = desc.FrontFace;

            Microsoft::WRL::ComPtr<ID3D11DepthStencilState> newState;
            RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
            ID3D11Device1* device = renderer->GetDevice();
            HRESULT hr = device->CreateDepthStencilState(&desc, &newState);
            if (SUCCEEDED(hr))
            {
                _depthStencilCache[key] = newState;
                state = newState.Get();
            }
        }

        if (state)
        {
            _context->OMSetDepthStencilState(state, stencilRef);
        }
    }

    void CommandListDX11::SetColorWriteMask(bool r, bool g, bool b, bool a)
    {
        UINT8 mask = 0;
        if (r)
            mask |= D3D11_COLOR_WRITE_ENABLE_RED;
        if (g)
            mask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
        if (b)
            mask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
        if (a)
            mask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

        // Chercher dans le cache
        auto it = _blendCache.find(mask);
        ID3D11BlendState* state = nullptr;

        if (it != _blendCache.end())
        {
            state = it->second.Get();
        }
        else
        {
            D3D11_BLEND_DESC desc = {};
            desc.RenderTarget[0].BlendEnable = FALSE;
            desc.RenderTarget[0].RenderTargetWriteMask = mask;

            Microsoft::WRL::ComPtr<ID3D11BlendState> newState;
            RendererDX11* renderer = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
            ID3D11Device1* device = renderer->GetDevice();
            HRESULT hr = device->CreateBlendState(&desc, &newState);
            if (SUCCEEDED(hr))
            {
                _blendCache[mask] = newState;
                state = newState.Get();
            }
        }

        if (state)
        {
            _context->OMSetBlendState(state, nullptr, 0xFFFFFFFF);
        }
    }
} // namespace Frost
