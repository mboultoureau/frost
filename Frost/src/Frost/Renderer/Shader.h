#pragma once

#include <unordered_map>
#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#include <memory>

namespace Frost
{
	class Shader
	{
	public:
		using Filepath = LPCWSTR;
		virtual ~Shader() = default;
	};

	class VertexShader : public Shader
	{
	public:
		void Create(const Filepath& filepath, const D3D11_INPUT_ELEMENT_DESC* inputLayout, UINT numElements);
		ID3D11InputLayout* GetInputLayout() const { return _inputLayout.Get(); }
		ID3D11VertexShader* Get() const { return _vertexShader.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;
	};

	class PixelShader : public Shader
	{
	public:
		void Create(const Filepath& filepath);
		ID3D11PixelShader* Get() const { return _pixelShader.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
	};


	class ShaderLibrary
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<Shader>> _shaders;
	};
}
