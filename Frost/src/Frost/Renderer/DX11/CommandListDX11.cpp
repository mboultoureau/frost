#include "Frost/Renderer/DX11/CommandListDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Renderer/DX11/BufferDX11.h"
#include "Frost/Renderer/DX11/SamplerDX11.h"

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
		}
	}


	void CommandListDX11::SetRenderTargets(uint32_t count, Texture** renderTargets, Texture* depthStencil)
	{
		std::vector<ID3D11RenderTargetView*> rtvs(count);
		for (uint32_t i = 0; i < count; ++i)
		{
			rtvs[i] = static_cast<TextureDX11*>(renderTargets[i])->GetRenderTargetView();
		}

		ID3D11DepthStencilView* dsv = depthStencil ? static_cast<TextureDX11*>(depthStencil)->GetDepthStencilView() : nullptr;

		_context->OMSetRenderTargets(count, rtvs.data(), dsv);
	}

	void CommandListDX11::ClearRenderTarget(Texture* renderTarget, const float color[4])
	{
		FT_ENGINE_ASSERT(renderTarget, "Render target cannot be null.");
		ID3D11RenderTargetView* rtv = static_cast<TextureDX11*>(renderTarget)->GetRenderTargetView();
		_context->ClearRenderTargetView(rtv, color);
	}

	void CommandListDX11::ClearRenderTarget(Texture* renderTarget, const float color[4], const Viewport viewport)
	{
		FT_ENGINE_ASSERT(renderTarget, "Render target cannot be null.");
		ID3D11RenderTargetView* rtv = static_cast<TextureDX11*>(renderTarget)->GetRenderTargetView();

		D3D11_RECT rect;
		rect.left = static_cast<LONG>(viewport.x * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetWidth()));
		rect.top = static_cast<LONG>(viewport.y * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetHeight()));
		rect.right = static_cast<LONG>((viewport.x + viewport.width) * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetWidth()));
		rect.bottom = static_cast<LONG>((viewport.y + viewport.height) * static_cast<float>(static_cast<TextureDX11*>(renderTarget)->GetHeight()));
		
		_context->ClearView(rtv, color, &rect, 1);
	}

	void CommandListDX11::ClearDepthStencil(Texture* depthStencil, bool clearDepth, float depthValue, bool clearStencil, uint8_t stencilValue)
	{
		FT_ENGINE_ASSERT(depthStencil, "Depth stencil cannot be null.");
		ID3D11DepthStencilView* dsv = static_cast<TextureDX11*>(depthStencil)->GetDepthStencilView();

		UINT clearFlags = 0;
		if (clearDepth) clearFlags |= D3D11_CLEAR_DEPTH;
		if (clearStencil) clearFlags |= D3D11_CLEAR_STENCIL;

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
		case ShaderType::Vertex:   _context->VSSetShader(static_cast<ID3D11VertexShader*>(shaderDX11->GetShaderObject()), nullptr, 0); break;
		case ShaderType::Pixel:    _context->PSSetShader(static_cast<ID3D11PixelShader*>(shaderDX11->GetShaderObject()), nullptr, 0); break;
		case ShaderType::Geometry: _context->GSSetShader(static_cast<ID3D11GeometryShader*>(shaderDX11->GetShaderObject()), nullptr, 0); break;
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
	}

	void CommandListDX11::SetTexture(const Texture* texture, uint32_t slot)
	{
		ID3D11ShaderResourceView* srv = nullptr;
		if (texture)
		{
			srv = static_cast<const TextureDX11*>(texture)->GetShaderResourceView();
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
		}

		_context->IASetPrimitiveTopology(dxTopology);
	}

	void CommandListDX11::Draw(uint32_t vertexCount, uint32_t startVertexLocation)
	{
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->Draw(vertexCount, startVertexLocation);
	}

	void CommandListDX11::DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation)
	{
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}
}
