#pragma once

#include "Frost/Core/Window.h"
#include "Frost/Renderer/Device.h"
#include "Frost/Renderer/Viewport.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Shader.h"

#include <memory>
#include <d3d11_1.h>

namespace Frost
{
	class Window;
	class Device;

	class Renderer
	{
	public:
		Renderer();
		
		void Init();

		void Present();
		void DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation);
		void ClearColor(float r, float g, float b, float a = 1.0f);
		void ClearColor(const Viewport& viewport, float r, float g, float b, float a = 1.0f);

		void SetViewport(const Viewport& viewport);
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void SetVertexBuffer(const VertexBuffer& buffer, UINT stride, UINT offset);
		void SetIndexBuffer(const IndexBuffer& buffer, UINT offset);
		void SetInputLayout(ID3D11InputLayout* inputLayout);

		void EnableVertexShader(VertexShader& vertexShader);
		void EnablePixelShader(PixelShader& pixelShader);

		void UpdateSubresource(ID3D11Buffer* buffer, const void* data, UINT dataSize);
		void SetVertexConstantBuffer(UINT slot, ID3D11Buffer* buffer);
		void SetPixelConstantBuffer(UINT slot, ID3D11Buffer* buffer);

		void SetGeometryShader();

		void CreateBuffer(const D3D11_BUFFER_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Buffer** buffer);
		void CreateVertexShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11VertexShader** vertexShader);
		void CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDescs, UINT numElements, const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11InputLayout** inputLayout);
		void CreatePixelShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11PixelShader** pixelShader);

		bool CreateTexture2D(const D3D11_TEXTURE2D_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Texture2D** texture);
		bool CreateShaderResourceView(ID3D11Resource* resource, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc, ID3D11ShaderResourceView** shaderResourceView);
		void SetPixelSampler(UINT slot, ID3D11SamplerState* samplerState);
		void SetPixelShaderResource(UINT slot, ID3D11ShaderResourceView* shaderResourceView);

		void CreateSamplerState(const D3D11_SAMPLER_DESC* samplerDesc, ID3D11SamplerState** samplerState);

		ID3D11Device* Get3DDevice() const;
		ID3D11DeviceContext1* GetImmediateContext() const;
		Device* GetDevice() const { return _device.get(); }

	private:
		std::shared_ptr<Window> _window;
		std::unique_ptr<Device> _device;
	};
}


