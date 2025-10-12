#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Buffer.h"

namespace Frost
{
    class RendererSystem : public System
    {
    public:
		RendererSystem();
        void Update(Frost::ECS& ecs, float deltaTime) override;

    private:
        void Render(ECS& ecs);
		
        VertexShader _vertexShader;
		PixelShader _pixelShader;
		ConstantBuffer _constantBuffer;
    };
}