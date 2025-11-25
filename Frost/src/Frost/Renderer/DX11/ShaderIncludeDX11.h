#pragma once

#include <d3dcompiler.h>
#include <filesystem>
#include <fstream>

namespace Frost
{
	class ShaderIncludeDX11 : public ID3DInclude
	{
	public:
		ShaderIncludeDX11(const std::string& shaderDir);
		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;
		HRESULT __stdcall Close(LPCVOID pData) override;

	private:
		std::filesystem::path _shaderDir;
	};
}
