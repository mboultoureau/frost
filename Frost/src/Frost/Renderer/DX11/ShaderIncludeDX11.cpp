#include "Frost/Renderer/DX11/ShaderIncludeDX11.h"
#include "Frost/Debugging/Logger.h"

namespace Frost
{
    ShaderIncludeDX11::ShaderIncludeDX11(const std::string& shaderDir) : _shaderDir(shaderDir)
    {
    }

    HRESULT __stdcall ShaderIncludeDX11::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
    {
        std::filesystem::path finalPath = _shaderDir / pFileName;

        std::ifstream file(finalPath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            FT_ENGINE_ERROR("Shader Include Error: Failed to open file: {0}", finalPath.string());
            return E_FAIL;
        }

        size_t fileSize = (size_t)file.tellg();
        char* buffer = new char[fileSize];

        file.seekg(0);
        file.read(buffer, fileSize);
        file.close();

        *ppData = buffer;
        *pBytes = (UINT)fileSize;

        return S_OK;
    }

    HRESULT __stdcall ShaderIncludeDX11::Close(LPCVOID pData)
    {
        char* buffer = (char*)pData;
        delete[] buffer;
        return S_OK;
    }
}
