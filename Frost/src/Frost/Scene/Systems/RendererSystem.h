#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Renderer/Buffer.h"
#include "Frost/Renderer/Frustum.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Scene/Components/HUD_Image.h"
#include "Frost/Scene/Components/UIButton.h"
#include <wrl/client.h>

namespace Frost
{
    // Structure de constantes 2D pour le HUD
    struct alignas(16) HUD_ShadersParams
    {
        DirectX::XMMATRIX world;
        DirectX::XMFLOAT4 screenDimensions;
    };
    // Structure de sommet 2D
    struct HUD_Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT2 TexCoord;
    };

    class RendererSystem : public System
    {
    public:
		RendererSystem();
        void LateUpdate(Frost::ECS& ecs, float deltaTime) override;

    private:
        void Render(ECS& ecs);
        void RenderHUD(ECS& ecs);
        void SetRendererToFilter(Material::FilterMode filterMode);
        void DrawImage(const HUD_Image& image, HUD_ShadersParams& hudSp);
		
        // VS & PS 3D
        VertexShader _vertexShader;
		PixelShader _pixelShader;
        // VS & PS 2D
        VertexShader _hudVertexShader;
        PixelShader _hudPixelShader;
        //Buffer 3D
		ConstantBuffer _constantBuffer;
        //Buffer 2D
        ConstantBuffer _hudConstantBuffer;

        VertexBuffer _hudVertexBuffer;
        IndexBuffer _hudIndexBuffer;

        Frustum _frustum;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerLinear;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerPoint;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> _samplerAnisotropic;

    };
}