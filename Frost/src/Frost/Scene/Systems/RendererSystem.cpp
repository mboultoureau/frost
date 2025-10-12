#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"

namespace Frost
{
	struct ShadersParams
	{
		DirectX::XMMATRIX worldViewProjection;
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

		DirectX::XMFLOAT3 lightDirection{ -10.0f, 10.0f, 10.0f };
		DirectX::XMFLOAT4 lightColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 ambientColor{ 0.2f, 0.2f, 0.2f, 1.0f };
		float specularPower{ 2 };

		DirectX::XMFLOAT3 cameraPosition{ 5.0f, 5.0f, 5.0f };
	};


	RendererSystem::RendererSystem()
    {
        constexpr D3D11_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        _vertexShader.Create(L"../Frost/resources/shaders/Vertex.hlsl", inputLayout, _countof(inputLayout));
        _pixelShader.Create(L"../Frost/resources/shaders/Pixel.hlsl");
		_constantBuffer.Create(nullptr, sizeof(ShadersParams));
	}

	void RendererSystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		Render(ecs);
	}

    void RendererSystem::Render(ECS& ecs)
    {
		const auto& cameras = ecs.GetDataArray<Camera>();
		const auto& cameraEntities = ecs.GetIndexMap<Camera>();

		for (size_t c = 0; c < cameras.size(); ++c)
		{
			const Camera& camera = cameras[c];
			GameObject::Id cameraId = cameraEntities[c];

			DirectX::XMMATRIX view;
			// We need to get the camera's world transform to compute the view matrix
			WorldTransform* cameraTransform = ecs.GetComponent<WorldTransform>(cameraId);
			if (cameraTransform)
			{
				DirectX::XMVECTOR eyePosition = DirectX::XMLoadFloat3(&cameraTransform->position);
				// Calculate Up direction with position and rotation
				DirectX::XMVECTOR upDirection = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
					DirectX::XMQuaternionRotationRollPitchYaw(
						DirectX::XMConvertToRadians(cameraTransform->rotation.x),
						DirectX::XMConvertToRadians(cameraTransform->rotation.y),
						DirectX::XMConvertToRadians(cameraTransform->rotation.z)
					)
				);

				// Calculate forward direction with position and rotation
				DirectX::XMVECTOR forwardDirection = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
					DirectX::XMQuaternionRotationRollPitchYaw(
						DirectX::XMConvertToRadians(cameraTransform->rotation.x),
						DirectX::XMConvertToRadians(cameraTransform->rotation.y),
						DirectX::XMConvertToRadians(cameraTransform->rotation.z)
					)
				);

				DirectX::XMVECTOR focusPosition = DirectX::XMVectorAdd(eyePosition, forwardDirection);
				view = DirectX::XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
			}
			else
			{
				view = DirectX::XMMatrixIdentity();
			}


			DirectX::XMMATRIX projection;

			if (camera.projectionType == Camera::ProjectionType::Perspective)
			{
				projection = DirectX::XMMatrixPerspectiveFovLH(
					DirectX::XMConvertToRadians(camera.perspectiveFOV),
					camera.viewport.width / camera.viewport.height,
					camera.nearClip,
					camera.farClip
				);
			}
			else
			{
				float viewWidth = camera.orthographicSize * (camera.viewport.width / camera.viewport.height);
				float viewHeight = camera.orthographicSize;
				projection = DirectX::XMMatrixOrthographicLH(
					viewWidth,
					viewHeight,
					camera.nearClip,
					camera.farClip
				);
			}

			RendererAPI::SetViewport(camera.viewport);
			if (camera.clearOnRender)
			{
				RendererAPI::ClearColor(camera.backgroundColor[0], camera.backgroundColor[1], camera.backgroundColor[2], camera.backgroundColor[3]);
			}

			const auto& renderers = ecs.GetDataArray<MeshRenderer>();
			const auto& rendererEntities = ecs.GetIndexMap<MeshRenderer>();

			for (size_t i = 0; i < renderers.size(); ++i)
			{
				const MeshRenderer& mesh = renderers[i];
				GameObject::Id id = rendererEntities[i];

				WorldTransform* transform = ecs.GetComponent<WorldTransform>(id);

				if (transform)
				{
					RendererAPI::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					const UINT stride = sizeof(Vertex);
					const UINT offset = 0;
					RendererAPI::SetVertexBuffer(mesh.mesh->GetVertexBuffer(), stride, offset);
					RendererAPI::SetIndexBuffer(mesh.mesh->GetIndexBuffer(), 0);
					RendererAPI::SetInputLayout(_vertexShader.GetInputLayout());
					RendererAPI::EnableVertexShader(_vertexShader);

					ShadersParams sp;
					DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, projection);
					DirectX::XMMATRIX matWorld = DirectX::XMMatrixTranspose(
						DirectX::XMMatrixScaling(transform->scale.x, transform->scale.y, transform->scale.z) *
						DirectX::XMMatrixRotationRollPitchYaw(
							DirectX::XMConvertToRadians(transform->rotation.x),
							DirectX::XMConvertToRadians(transform->rotation.y),
							DirectX::XMConvertToRadians(transform->rotation.z)
						) *
						DirectX::XMMatrixTranslation(transform->position.x, transform->position.y, transform->position.z)
					);
					sp.worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(matWorld, viewProj));
					sp.world = matWorld;

					RendererAPI::UpdateSubresource(_constantBuffer.Get(), &sp, sizeof(ShadersParams));
					RendererAPI::SetVertexConstantBuffer(0, _constantBuffer.Get());
					RendererAPI::SetGeometryShader();
					RendererAPI::EnablePixelShader(_pixelShader);
					RendererAPI::SetPixelConstantBuffer(0, _constantBuffer.Get());
					RendererAPI::DrawIndexed(mesh.mesh->GetIndexBuffer().GetCount(), 0, 0);
				}
			}
        }
    }
}

