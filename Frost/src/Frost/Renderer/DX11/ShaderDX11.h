#pragma once

#include "Frost/Renderer/Shader.h"

#include <d3d11.h>
#include <vector>
#include <wrl/client.h>

namespace Frost
{
    class ShaderDX11 : public Shader
    {
    public:
        ShaderDX11(const ShaderDesc& desc);
        virtual ~ShaderDX11() override = default;

        const std::vector<char>& GetBytecode() const;
        ID3D11DeviceChild* GetShaderObject() const;

    private:
        Microsoft::WRL::ComPtr<ID3DBlob> _blob;
        Microsoft::WRL::ComPtr<ID3D11DeviceChild> _shader;
    };
} // namespace Frost