#pragma once

#include "Frost/Renderer/InputLayout.h"

#include <cstdint>
#include <d3d11.h>
#include <map>
#include <wrl/client.h>

namespace Frost
{
    class InputLayoutDX11 : public InputLayout
    {
    public:
        InputLayoutDX11(const VertexAttributeArray& attributes, const Shader& shader);
        virtual ~InputLayoutDX11() = default;

        ID3D11InputLayout* GetLayout() const;
        const std::map<uint32_t, uint32_t>& GetVertexStrides() const;

    private:
        Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;
        std::map<uint32_t, uint32_t> _vertexStrides;
    };
} // namespace Frost