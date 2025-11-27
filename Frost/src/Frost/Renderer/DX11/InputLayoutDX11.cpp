#include "Frost/Renderer/DX11/InputLayoutDX11.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/FormatDX11.h"
#include "Frost/Renderer/DX11/RendererDX11.h"
#include "Frost/Renderer/DX11/ShaderDX11.h"
#include "Frost/Renderer/RendererAPI.h"

#include <sstream>
#include <vector>

namespace Frost
{
    InputLayoutDX11::InputLayoutDX11(const VertexAttributeArray& attributes, const Shader& shader) :
        InputLayout(attributes, shader)
    {
        RendererDX11* rendererDX11 = dynamic_cast<RendererDX11*>(RendererAPI::GetRenderer());

        ID3D11Device* device = rendererDX11->GetDevice();
        const ShaderDX11* d3d11Shader = dynamic_cast<const ShaderDX11*>(&shader);

        FT_ENGINE_ASSERT(device, "InputLayoutDX11: Invalid D3D11 Device.");
        FT_ENGINE_ASSERT(d3d11Shader,
                         "InputLayoutDX11: Provided shader is not a valid "
                         "DirectX 11 shader.");
        FT_ENGINE_ASSERT(shader.GetType() == ShaderType::Vertex,
                         "InputLayoutDX11: Shader provided is not a vertex shader.");

        const auto& bytecode = d3d11Shader->GetBytecode();
        FT_ENGINE_ASSERT(!bytecode.empty(), "InputLayoutDX11: Shader bytecode is empty.");

        std::vector<D3D11_INPUT_ELEMENT_DESC> elementDescs;
        for (const auto& attribute : attributes)
        {
            for (uint32_t i = 0; i < attribute.arraySize; i++)
            {
                D3D11_INPUT_ELEMENT_DESC desc = {};
                desc.SemanticName = attribute.name.c_str();
                desc.SemanticIndex = i;
                desc.Format = ToDXGIFormat(attribute.format);
                desc.InputSlot = attribute.bufferIndex;

                desc.AlignedByteOffset = attribute.offset + (i * GetFormatSize(attribute.format));

                desc.InputSlotClass =
                    attribute.isInstanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                desc.InstanceDataStepRate = attribute.isInstanced ? 1 : 0;

                elementDescs.push_back(desc);
            }

            if (_vertexStrides.find(attribute.bufferIndex) == _vertexStrides.end())
            {
                _vertexStrides[attribute.bufferIndex] = attribute.elementStride;
            }
            else
            {
                FT_ENGINE_ASSERT(_vertexStrides[attribute.bufferIndex] == attribute.elementStride,
                                 "InputLayoutDX11: Mismatched element strides.");
            }
        }

        HRESULT hr = device->CreateInputLayout(elementDescs.data(),
                                               static_cast<UINT>(elementDescs.size()),
                                               bytecode.data(),
                                               bytecode.size(),
                                               &_inputLayout);

        FT_ENGINE_ASSERT(SUCCEEDED(hr), "InputLayoutDX11: Failed to create input layout for shader.");
    }

    ID3D11InputLayout* InputLayoutDX11::GetLayout() const
    {
        return _inputLayout.Get();
    }

    const std::map<uint32_t, uint32_t>& InputLayoutDX11::GetVertexStrides() const
    {
        return _vertexStrides;
    }
} // namespace Frost