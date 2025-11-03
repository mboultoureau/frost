#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Buffer.h"

#include <wrl/client.h>

namespace Frost
{
    class RendererSystem : public System
    {
    public:
		RendererSystem();
        void LateUpdate(Frost::ECS& ecs, float deltaTime) override;

    private:
        void Render(ECS& ecs);
		
        VertexShader _vertexShader;
		PixelShader _pixelShader;
		ConstantBuffer _constantBuffer;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerState;
    };
}