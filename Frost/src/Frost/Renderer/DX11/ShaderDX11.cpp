#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/ShaderIncludeDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include <d3dcompiler.h>
#include <filesystem>
#include <fstream>

namespace Frost
{
    // Helper function to get the shader target profile string
    static const char* GetShaderTarget(ShaderType type)
    {
        switch (type)
        {
            case ShaderType::Compute:
                return "cs_5_0";
            case ShaderType::Domain:
                return "ds_5_0";
            case ShaderType::Geometry:
                return "gs_5_0";
            case ShaderType::Hull:
                return "hs_5_0";
            case ShaderType::Pixel:
                return "ps_5_0";
            case ShaderType::Vertex:
                return "vs_5_0";
        }
        FT_ENGINE_ASSERT(false, "Unknown shader type!");
        return "";
    }

    ShaderDX11::ShaderDX11(const ShaderDesc& desc) : Shader(desc)
    {
        RendererDX11* rendererDX11 = static_cast<RendererDX11*>(RendererAPI::GetRenderer());
        ID3D11Device* device = rendererDX11->GetDevice();

        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef FT_DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif

        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

        std::filesystem::path shaderPath(desc.filePath);
        std::filesystem::path absPath = std::filesystem::absolute(shaderPath);

        if (!std::filesystem::exists(shaderPath))
        {
            std::filesystem::path currentPath = std::filesystem::current_path();

            FT_ENGINE_CRITICAL("SHADER FILE NOT FOUND!");
            FT_ENGINE_CRITICAL("Looking for: {0}", desc.filePath);
            FT_ENGINE_CRITICAL("Resolved Absolute Path: {0}", absPath.string());
            FT_ENGINE_CRITICAL("Current Working Directory: {0}", currentPath.string());

            FT_ENGINE_ASSERT(false, "Shader file not found on disk");
            return;
        }

        std::string shaderDir = shaderPath.parent_path().string();
        std::wstring wideFilePath = shaderPath.wstring();

        ShaderIncludeDX11 includeHandler(shaderDir);

        HRESULT hr = D3DCompileFromFile(wideFilePath.c_str(),
                                        nullptr,
                                        &includeHandler,
                                        "main",
                                        GetShaderTarget(desc.type),
                                        flags,
                                        0,
                                        &_blob,
                                        &errorBlob);

        if (FAILED(hr))
        {
            if (errorBlob)
            {
                const char* errorMessage = static_cast<const char*>(errorBlob->GetBufferPointer());
                FT_ENGINE_CRITICAL("Shader compilation failed for file: {0}\n--- DETAILS "
                                   "---\n{1}\n------------",
                                   desc.filePath,
                                   errorMessage);
            }
            else
            {
                FT_ENGINE_CRITICAL("Failed to find or read shader file: {0}. HRESULT: 0x{1:x}", desc.filePath, hr);
            }

            FT_ENGINE_ASSERT(false, "Shader compilation failed!");
            return;
        }

        if (errorBlob)
        {
            const char* warningMessage = static_cast<const char*>(errorBlob->GetBufferPointer());
            FT_ENGINE_WARN("Shader compiled with warnings for file: {0}\n{1}", desc.filePath, warningMessage);
        }

        const void* bytecodePtr = _blob->GetBufferPointer();
        const SIZE_T bytecodeSize = _blob->GetBufferSize();

        switch (desc.type)
        {
            case ShaderType::Vertex:
                hr = device->CreateVertexShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11VertexShader**)_shader.GetAddressOf());
                break;
            case ShaderType::Pixel:
                hr = device->CreatePixelShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11PixelShader**)_shader.GetAddressOf());
                break;
            case ShaderType::Hull:
                hr = device->CreateHullShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11HullShader**)_shader.GetAddressOf());
                break;
            case ShaderType::Domain:
                hr = device->CreateDomainShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11DomainShader**)_shader.GetAddressOf());
                break;
            case ShaderType::Geometry:
                hr = device->CreateGeometryShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11GeometryShader**)_shader.GetAddressOf());
                break;
            case ShaderType::Compute:
                hr = device->CreateComputeShader(
                    bytecodePtr, bytecodeSize, nullptr, (ID3D11ComputeShader**)_shader.GetAddressOf());
                break;
        }

        FT_ENGINE_ASSERT(SUCCEEDED(hr), "Failed to create D3D11 shader object!");
    }

    const std::vector<char>& ShaderDX11::GetBytecode() const
    {
        static std::vector<char> bytecode;
        if (_blob)
        {
            const char* data = static_cast<const char*>(_blob->GetBufferPointer());
            bytecode.assign(data, data + _blob->GetBufferSize());
        }
        return bytecode;
    }

    ID3D11DeviceChild* ShaderDX11::GetShaderObject() const
    {
        return _shader.Get();
    }
} // namespace Frost