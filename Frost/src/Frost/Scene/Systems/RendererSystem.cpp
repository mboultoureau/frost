#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/ModelRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"

namespace Frost
{
	// The shader must be align as 16 bytes
	struct alignas(16) ShadersParams
	{
		DirectX::XMMATRIX worldViewProjection;
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

		DirectX::XMFLOAT4 lightPosition{ -10.0f, 10.0f, 10.0f, 1.0f };
		DirectX::XMFLOAT4 ambientColor{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 diffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };

		DirectX::XMFLOAT4 cameraPosition{ 0.0f, 0.0f, -10.0f, 1.0f };

		int numberDiffuseTextures = 0;
		int padding[3];
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

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		RendererAPI::CreateSamplerState(&samplerDesc, _samplerState.GetAddressOf());
	}

	void RendererSystem::LateUpdate(Frost::ECS& ecs, float deltaTime)
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

			DirectX::XMVECTOR cameraRotationQuat = DirectX::XMLoadFloat4(&cameraTransform->rotation);

			// Calculate up vector from rotation
			DirectX::XMVECTOR worldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR worldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

			DirectX::XMVECTOR cameraUp = DirectX::XMVector3Rotate(worldUp, cameraRotationQuat);
			DirectX::XMVECTOR cameraDirection = DirectX::XMVector3Rotate(worldForward, cameraRotationQuat);

			view = DirectX::XMMatrixLookToLH(
				DirectX::XMLoadFloat3(&cameraTransform->position),
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

			const auto& renderers = ecs.GetDataArray<ModelRenderer>();
			const auto& rendererEntities = ecs.GetIndexMap<ModelRenderer>();

			for (size_t i = 0; i < renderers.size(); ++i)
			{
				const ModelRenderer& modelRenderer = renderers[i];
				GameObject::Id id = rendererEntities[i];

				WorldTransform* transform = ecs.GetComponent<WorldTransform>(id);

				if (transform && modelRenderer.model)
				{
					DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(
						transform->scale.x,
						transform->scale.y,
						transform->scale.z
					);

					// 2. Create the Rotation Matrix from the quaternion
					// This replaces the incorrect XMMatrixRotationRollPitchYaw call
					DirectX::XMMATRIX matRotation = DirectX::XMMatrixRotationQuaternion(
						DirectX::XMLoadFloat4(&transform->rotation)
					);

					// 3. Create the Translation Matrix
					DirectX::XMMATRIX matTranslation = DirectX::XMMatrixTranslation(
						transform->position.x,
						transform->position.y,
						transform->position.z
					);

					// 4. Combine into World Matrix (Scale * Rotation * Translation)
					DirectX::XMMATRIX matWorld = matScale * matRotation * matTranslation;

					RendererAPI::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					RendererAPI::SetInputLayout(_vertexShader.GetInputLayout());
					RendererAPI::EnableVertexShader(_vertexShader);
					RendererAPI::SetGeometryShader();
					RendererAPI::EnablePixelShader(_pixelShader);
					RendererAPI::SetPixelSampler(0, _samplerState.Get());

					const UINT stride = sizeof(Vertex);
					const UINT offset = 0;

					ShadersParams sp = {};
					DirectX::XMMATRIX viewProj = view * projection;
					sp.worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(matWorld, viewProj));
					sp.world = XMMatrixTranspose(matWorld);
					sp.cameraPosition = actualCameraPosition;

					const Model& model = *modelRenderer.model;
					for (const Mesh& mesh : model.GetMeshes())
					{
						const Material& material = model.GetMaterials()[mesh.GetMaterialIndex()];
						sp.diffuseColor = { material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z, 1.0f };

						if (material.diffuseTextures.size() > 0)
						{
							sp.numberDiffuseTextures = 1;
							RendererAPI::SetPixelShaderResource(0, material.diffuseTextures[0]->GetTextureView());
						}
						else
						{
							sp.numberDiffuseTextures = 0;
							RendererAPI::SetPixelShaderResource(0, nullptr);
						}

						RendererAPI::UpdateSubresource(_constantBuffer.Get(), &sp, sizeof(ShadersParams));
						RendererAPI::SetVertexConstantBuffer(0, _constantBuffer.Get());
						RendererAPI::SetPixelConstantBuffer(0, _constantBuffer.Get());

						const UINT stride = sizeof(Vertex);
						const UINT offset = 0;

						RendererAPI::SetVertexBuffer(mesh.GetVertexBuffer(), stride, offset);
						RendererAPI::SetIndexBuffer(mesh.GetIndexBuffer(), 0);
						
						RendererAPI::DrawIndexed(mesh.GetIndexCount(), 0, 0);
					}
				}
			}
        }
    }
}

