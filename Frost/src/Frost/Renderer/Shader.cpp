#include "Shader.h"

#include "Frost/Renderer/RendererAPI.h"

#include <d3dcompiler.h>

namespace Frost
{
	class CantCompileShader{};

	void VertexShader::Create(const Filepath& filepath, const D3D11_INPUT_ELEMENT_DESC* inputLayout, UINT numElements)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> ppCode;

		HRESULT result = D3DCompileFromFile(
			filepath,
			nullptr,
			nullptr,
			"VSMain",
			"vs_5_0",
			D3DCOMPILE_ENABLE_STRICTNESS,
			0,
			&ppCode,
			nullptr
		);

		if (FAILED(result))
		{
			MessageBoxA(nullptr, "Failed to compile vertex shader!", "Error", MB_OK | MB_ICONERROR);
			throw CantCompileShader();
		}

		RendererAPI::Get().CreateVertexShader(
			ppCode->GetBufferPointer(),
			ppCode->GetBufferSize(),
			&_vertexShader
		);

		RendererAPI::Get().CreateInputLayout(
			inputLayout,
			numElements,
			ppCode->GetBufferPointer(),
			ppCode->GetBufferSize(),
			&_inputLayout
		);
	}

	void PixelShader::Create(const Filepath& filepath)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> ppCode;
		
		HRESULT result = D3DCompileFromFile(
			filepath,
			nullptr,
			nullptr,
			"PSMain",
			"ps_5_0",
			D3DCOMPILE_ENABLE_STRICTNESS,
			0,
			&ppCode,
			nullptr
		);

		if (FAILED(result))
		{
			MessageBoxA(nullptr, "Failed to compile pixel shader!", "Error", MB_OK | MB_ICONERROR);
			throw CantCompileShader();
		}

		RendererAPI::Get().CreatePixelShader(
			ppCode->GetBufferPointer(),
			ppCode->GetBufferSize(),
			&_pixelShader
		);
	}
}

