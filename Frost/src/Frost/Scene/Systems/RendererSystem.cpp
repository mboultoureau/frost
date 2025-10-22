#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/MeshRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"

namespace Frost
{
	struct ShadersParams
	{
		DirectX::XMMATRIX worldViewProjection;
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

		DirectX::XMFLOAT4 lightPosition{ -10.0f, 10.0f, 10.0f, 1.0f };
		DirectX::XMFLOAT4 ambientColor{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 diffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT4 cameraPosition{ 0.0f, 0.0f, -10.0f, 1.0f };
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
		RendererAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		const WindowDimensions currentWindowDimensions = Application::Get().GetWindow()->GetDimensions();
		if (currentWindowDimensions.width == 0 || currentWindowDimensions.height == 0)
		{
			return;
		}

		const auto& cameras = ecs.GetDataArray<Camera>();
		const auto& cameraEntities = ecs.GetIndexMap<Camera>();

		for (size_t c = 0; c < cameras.size(); ++c)
		{
			const Camera& camera = cameras[c];
			GameObject::Id cameraId = cameraEntities[c];

			const WorldTransform* cameraTransform = ecs.GetComponent<WorldTransform>(cameraId);
			if (!cameraTransform)
			{
				continue;
			}

			// Calculate viewport
			float width = camera.viewport.width * currentWindowDimensions.width;
			float height = camera.viewport.height * currentWindowDimensions.height;
			float viewportAspectRatio = width / height;

			DirectX::XMMATRIX view;
			DirectX::XMFLOAT4 actualCameraPosition = { cameraTransform->position.x, cameraTransform->position.y, cameraTransform->position.z, 1.0f };

			// Calculate up vector from rotation
			DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			DirectX::XMMATRIX cameraRotationMatrix = 
				DirectX::XMMatrixRotationRollPitchYaw(
					Angle<Radian>(cameraTransform->rotation.x).value(),
					Angle<Radian>(cameraTransform->rotation.y).value(),
					Angle<Radian>(cameraTransform->rotation.z).value()
				);

			DirectX::XMVECTOR cameraUp = DirectX::XMVector3TransformNormal(worldUp, cameraRotationMatrix);

			// Calculate forward vector from rotation
			DirectX::XMVECTOR worldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			DirectX::XMVECTOR cameraDirection = DirectX::XMVector3TransformNormal(worldForward, cameraRotationMatrix);

			view = DirectX::XMMatrixLookToLH(
				DirectX::XMVectorSet(actualCameraPosition.x, actualCameraPosition.y, actualCameraPosition.z, 1.0f),
				cameraDirection,
				cameraUp
			);

			DirectX::XMMATRIX projection;
			if (camera.projectionType == Camera::ProjectionType::Orthographic)
			{
				float orthoWidth = camera.orthographicSize * viewportAspectRatio;
				float orthoHeight = camera.orthographicSize;
				projection = DirectX::XMMatrixOrthographicLH(
					orthoWidth * 2.0f,
					orthoHeight * 2.0f,
					camera.nearClip,
					camera.farClip
				);
			}
			else
			{
				projection = DirectX::XMMatrixPerspectiveFovLH(
					Angle<Radian>(camera.perspectiveFOV).value(),
					viewportAspectRatio,
					camera.nearClip,
					camera.farClip
				);
			}

			RendererAPI::SetViewport(camera.viewport);

			if (camera.clearOnRender)
			{
				RendererAPI::ClearColor(camera.viewport, camera.backgroundColor[0], camera.backgroundColor[1], camera.backgroundColor[2], camera.backgroundColor[3]);
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

					ShadersParams sp = {};
					DirectX::XMMATRIX viewProj = view * projection;
					DirectX::XMMATRIX matWorld = DirectX::XMMatrixIdentity();
					matWorld = DirectX::XMMatrixTranslation(transform->position.x, transform->position.y, transform->position.z);

					sp.worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(matWorld, viewProj));
					sp.world = XMMatrixTranspose(matWorld);
					sp.cameraPosition = actualCameraPosition;

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

