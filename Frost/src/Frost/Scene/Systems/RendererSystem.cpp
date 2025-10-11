#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/RendererAPI.h"

namespace Frost
{
	RendererSystem::RendererSystem()
    {
        constexpr D3D11_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        _vertexShader.Create(L"resources/shaders/Vertex.hlsl", inputLayout, _countof(inputLayout));
        _pixelShader.Create(L"resources/shaders/Pixel.hlsl");
	}

	void RendererSystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		Render(ecs);
	}

    void RendererSystem::Render(ECS& ecs)
    {
        const auto& renderers = ecs.GetDataArray<MeshRenderer>();
        const auto& rendererEntities = ecs.GetIndexMap<MeshRenderer>();

        for (size_t i = 0; i < renderers.size(); ++i)
        {
            const MeshRenderer& mesh = renderers[i];
            GameObject::Id id = rendererEntities[i];

            WorldTransform* transform = ecs.GetComponent<WorldTransform>(id);

            if (transform)
            {
				/*
				// Obtenir le contexte
				ID3D11DeviceContext* pImmediateContext = pDevice->GetImmediateContext();
				// Choisir la topologie des primitives
				pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				// Source des sommets
				const UINT stride = sizeof(CSommetBloc);
				const UINT offset = 0;
				pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
				// Source des index
				pImmediateContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
				// input layout des sommets
				pImmediateContext->IASetInputLayout(pVertexLayout);
				// Activer le VS
				pImmediateContext->VSSetShader(pVertexShader, nullptr, 0);
				// Initialiser et sélectionner les « constantes » du VS
				ShadersParams sp;
				XMMATRIX viewProj = CMoteurWindows::GetInstance().GetMatViewProj();

				sp.matWorldViewProj = XMMatrixTranspose(matWorld * viewProj);
				sp.matWorld = XMMatrixTranspose(matWorld);

				sp.vLumiere = XMVectorSet(-10.0f, 10.0f, -10.0f, 1.0f);
				sp.vCamera = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
				sp.vAEcl = XMVectorSet(0.2f, 0.2f, 0.2f, 1.0f);
				sp.vAMat = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
				sp.vDEcl = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
				sp.vDMat = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
				pImmediateContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &sp, 0,
					0);
				pImmediateContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
				// Pas de Geometry Shader
				pImmediateContext->GSSetShader(nullptr, nullptr, 0);
				// Activer le PS
				pImmediateContext->PSSetShader(pPixelShader, nullptr, 0);
				pImmediateContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);



				// **** Rendu de l’objet
				pImmediateContext->DrawIndexed(ARRAYSIZE(index_bloc), 0, 0);


				*/



            }
        }
    }
}

