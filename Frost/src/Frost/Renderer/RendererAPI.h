#pragma once

#include "Frost/Renderer/Renderer.h"
#include "Frost/Scene/Components/ModelRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"

#include <d3d11_1.h>

namespace Frost
{
	class RendererAPI
	{
	public:
		static void ClearColor(float r, float g, float b, float a = 1.0f);
		static void ClearColor(const Viewport& viewport, float r, float g, float b, float a = 1.0f);
		static void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation);
		static void Present();

		static void SetViewport(const Viewport& viewport);
		static void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		static void SetVertexBuffer(const VertexBuffer& buffer, UINT stride, UINT offset);
		static void SetIndexBuffer(const IndexBuffer& buffer, UINT offset);
		static void SetInputLayout(ID3D11InputLayout* inputLayout);

		static void EnableVertexShader(VertexShader& vertexShader);
		static void EnablePixelShader(PixelShader& pixelShader);

		static void UpdateSubresource(ID3D11Buffer* buffer, const void* data, UINT dataSize);
		static void SetVertexConstantBuffer(UINT slot, ID3D11Buffer* buffer);
		static void SetPixelConstantBuffer(UINT slot, ID3D11Buffer* buffer);

		static bool CreateTexture2D(const D3D11_TEXTURE2D_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Texture2D** texture);
		static bool CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc, ID3D11ShaderResourceView** shaderResourceView);
		static void CreateSamplerState(const D3D11_SAMPLER_DESC* samplerDesc, ID3D11SamplerState** samplerState);
		static void SetPixelSampler(UINT slot, ID3D11SamplerState* samplerState);
		static void SetPixelShaderResource(UINT slot, ID3D11ShaderResourceView* shaderResourceView);

		static void SetGeometryShader();

		static ID3D11Device* Get3DDevice();
		static ID3D11DeviceContext1* GetImmediateContext();

		static Renderer& Get();
	};
}