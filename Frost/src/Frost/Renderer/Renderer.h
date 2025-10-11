#pragma once

#include "Frost/Core/Window.h"
#include "Frost/Renderer/Device.h"

#include <memory>
#include <d3d11.h>

namespace Frost
{
	class Window;
	class Device;

	class Renderer
	{
	public:
		Renderer();
		void Init();
		void DrawFrame();
		void ClearColor(float r, float g, float b, float a = 1.0f);

		void CreateBuffer(const D3D11_BUFFER_DESC* descriptor, const D3D11_SUBRESOURCE_DATA* subresourceData, ID3D11Buffer** buffer);
		void CreateVertexShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11VertexShader** vertexShader);
		void CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* inputElementDescs, UINT numElements, const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11InputLayout** inputLayout);
		void CreatePixelShader(const void* shaderBytecode, SIZE_T bytecodeLength, ID3D11PixelShader** pixelShader);

	private:
		std::shared_ptr<Window> _window;
		std::unique_ptr<Device> _device;
	};
}


