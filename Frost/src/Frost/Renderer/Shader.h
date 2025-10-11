#pragma once

#include <unordered_map>
#include <string>
#include <wrl/client.h>
#include <d3d11.h>

namespace Frost
{
	class Shader
	{
	public:
		using Filepath = LPCWSTR;
	};

	class VertexShader : public Shader
	{
	public:
		void Create(const Filepath& filepath, const D3D11_INPUT_ELEMENT_DESC* inputLayout, UINT numElements);

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;
	};

	class PixelShader : public Shader
	{
	public:
		void Create(const Filepath& filepath);

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
	};


	class ShaderLibrary
	{
	private:
		std::unordered_map<std::string, Shader> _shaders;
	};
}
