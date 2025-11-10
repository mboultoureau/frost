#include "Frost/Scene/Systems/RendererSystem.h"

#include "Frost/Scene/Components/Camera.h"
#include "Frost/Scene/Components/ModelRenderer.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/Shader.h"

namespace Frost
{
	// Structures de constantes 3D
	struct alignas(16) ShadersParams
	{
		DirectX::XMMATRIX worldViewProjection;
		DirectX::XMMATRIX world = DirectX::XMMatrixIdentity();

		DirectX::XMFLOAT4 lightPosition{ -10.0f, 10.0f, 10.0f, 1.0f };
		DirectX::XMFLOAT4 ambientColor{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 diffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4 emissiveColor{ 0.0f , 0.0f , 0.0f , 1.0f };
		DirectX::XMFLOAT4 cameraPosition{ 0.0f, 0.0f, -10.0f, 1.0f };
		DirectX::XMFLOAT2 uvTiling{ 1.0f, 1.0f };
		DirectX::XMFLOAT2 uvOffset{ 0.0f, 0.0f };


		int numberDiffuseTextures = 0;
		int hasNormalMap = 0;
		int hasEmissiveTexture = 0;
		int hasAmbientOclusionTexture = 0;

		int hasMetallicTexture = 0;
		float roughnessValue = 0.5f;
		int hasRoughnessTexture = 0;
		int padding[1];
	};


	RendererSystem::RendererSystem() : _frustum{}
	{
		// shaders
		constexpr D3D11_INPUT_ELEMENT_DESC inputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		_vertexShader.Create(L"../Frost/resources/shaders/Vertex.hlsl", inputLayout, _countof(inputLayout));
		_pixelShader.Create(L"../Frost/resources/shaders/Pixel.hlsl");
		_constantBuffer.Create(nullptr, sizeof(ShadersParams));

		// HUD shaders
		HUD_Vertex vertices[4] =
		{
			{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f} },
			{ {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} },
			{ {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} },
			{ {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f} }
		};
		UINT indices[6] = { 0, 1, 2, 2, 1, 3 };

		_hudVertexBuffer.Create(vertices, sizeof(vertices)); 

		_hudIndexBuffer.Create(indices,sizeof(indices), 6);

		constexpr D3D11_INPUT_ELEMENT_DESC hudInputLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		_hudVertexShader.Create(L"../Frost/resources/shaders/HUD_Vertex.hlsl", hudInputLayout, _countof(hudInputLayout));
		_hudPixelShader.Create(L"../Frost/resources/shaders/HUD_Pixel.hlsl");

		// Texture sampler
		D3D11_SAMPLER_DESC pointDesc = {};
		pointDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		pointDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		pointDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
		pointDesc.MipLODBias = 0.0f;
		pointDesc.MinLOD = 0;
		pointDesc.MaxLOD = D3D11_FLOAT32_MAX;
		RendererAPI::CreateSamplerState(&pointDesc, _samplerPoint.GetAddressOf());

		D3D11_SAMPLER_DESC linearDesc = {};
		linearDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		linearDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		linearDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		linearDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		linearDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
		linearDesc.MipLODBias = 0.0f;
		linearDesc.MinLOD = 0;
		linearDesc.MaxLOD = D3D11_FLOAT32_MAX;
		RendererAPI::CreateSamplerState(&linearDesc, _samplerLinear.GetAddressOf());


		D3D11_SAMPLER_DESC anisDesc = {};
		anisDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		anisDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		anisDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		anisDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		anisDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
		anisDesc.MipLODBias = 0.0f;
		anisDesc.MinLOD = 0;
		anisDesc.MaxLOD = D3D11_FLOAT32_MAX;
		RendererAPI::CreateSamplerState(&anisDesc, _samplerAnisotropic.GetAddressOf());
	}

	void RendererSystem::LateUpdate(Frost::ECS& ecs, float deltaTime)
	{
		Render(ecs);
		RenderHUD(ecs);
	}

	void RendererSystem::Render(ECS& ecs)
	{
		RendererAPI::ClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		const WindowDimensions currentWindowDimensions = Application::Get().GetWindow()->GetRenderedZoneDimensions();
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

			float width = camera.viewport.width * currentWindowDimensions.width;
			float height = camera.viewport.height * currentWindowDimensions.height;
			float viewportAspectRatio = width / height;

			DirectX::XMMATRIX view;
			DirectX::XMFLOAT4 actualCameraPosition = { cameraTransform->position.x, cameraTransform->position.y, cameraTransform->position.z, 1.0f };

			DirectX::XMVECTOR cameraRotationQuat = DirectX::XMLoadFloat4(&cameraTransform->rotation);

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

					DirectX::XMMATRIX matRotation = DirectX::XMMatrixRotationQuaternion(
						DirectX::XMLoadFloat4(&transform->rotation)
					);

					DirectX::XMMATRIX matTranslation = DirectX::XMMatrixTranslation(
						transform->position.x,
						transform->position.y,
						transform->position.z
					);

					DirectX::XMMATRIX matWorld = matScale * matRotation * matTranslation;

					RendererAPI::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					RendererAPI::SetInputLayout(_vertexShader.GetInputLayout());
					RendererAPI::EnableVertexShader(_vertexShader);
					RendererAPI::SetGeometryShader();
					RendererAPI::EnablePixelShader(_pixelShader);

					const UINT stride = sizeof(Vertex);
					const UINT offset = 0;

					ShadersParams sp = {};
					DirectX::XMMATRIX viewProj = view * projection;
					sp.worldViewProjection = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(matWorld, viewProj));
					sp.world = XMMatrixTranspose(matWorld);
					sp.cameraPosition = actualCameraPosition;

					if (camera.frustumCulling) _frustum.Extract(viewProj, camera.frustumPadding);

					const Model& model = *modelRenderer.model;
					for (const Mesh& mesh : model.GetMeshes())
					{
						if (camera.frustumCulling)
						{
							BoundingBox worldBox = BoundingBox::TransformAABB(mesh.GetBoundingBox(), matWorld);
							if (!_frustum.IsInside(worldBox))
							{
								continue;
							}
						}

						const Material& material = model.GetMaterials()[mesh.GetMaterialIndex()];
						SetRendererToFilter(material.filterMode);

						sp.diffuseColor = { material.diffuseColor.x, material.diffuseColor.y, material.diffuseColor.z, 1.0f };

						if (material.diffuseTextures.size() > 0)
						{
							sp.numberDiffuseTextures = 1;

							TextureDX11* diffuseTexture = static_cast<TextureDX11*>(material.diffuseTextures[0].get());
							RendererAPI::SetPixelShaderResource(0, diffuseTexture->GetTextureView());
						}
						else
						{
							sp.numberDiffuseTextures = 0;
							RendererAPI::SetPixelShaderResource(0, nullptr);
						}

						if (material.normalTextures.size() > 0)
						{
							sp.hasNormalMap = 1;
							TextureDX11* normalTexture = static_cast<TextureDX11*>(material.normalTextures[0].get());
							RendererAPI::SetPixelShaderResource(1, normalTexture->GetTextureView());
						}
						else
						{
							sp.hasNormalMap = 0;
							RendererAPI::SetPixelShaderResource(1, nullptr);
						}

						sp.emissiveColor = { material.emissiveColor.x, material.emissiveColor.y, material.emissiveColor.z, 1.0f };

						if (material.emissiveTextures.size() > 0)
						{
							sp.hasEmissiveTexture = 1;
							TextureDX11* emissiveTexture = static_cast<TextureDX11*>(material.emissiveTextures[0].get());
							RendererAPI::SetPixelShaderResource(2, emissiveTexture->GetTextureView());
						}
						else
						{
							sp.hasEmissiveTexture = 0;
							RendererAPI::SetPixelShaderResource(2, nullptr);
						}

						if (material.ambientOclusionTextures.size() > 0)
						{
							sp.hasAmbientOclusionTexture = 1;
							TextureDX11* aoTexture = static_cast<TextureDX11*>(material.ambientOclusionTextures[0].get());
							RendererAPI::SetPixelShaderResource(3, aoTexture->GetTextureView());
						}
						else
						{
							sp.hasAmbientOclusionTexture = 0;
							RendererAPI::SetPixelShaderResource(3, nullptr);
						}

						if (material.metallicTextures.size() > 0)
						{
							sp.hasMetallicTexture = 1;
							TextureDX11* metallicTexture = static_cast<TextureDX11*>(material.metallicTextures[0].get());
							RendererAPI::SetPixelShaderResource(4, metallicTexture->GetTextureView());
						}
						else
						{
							sp.hasMetallicTexture = 0;
							RendererAPI::SetPixelShaderResource(4, nullptr);
						}

						sp.roughnessValue = material.roughnessValue;
						if (material.roughnessTextures.size() > 0)
						{
							sp.hasRoughnessTexture = 1;
							TextureDX11* roughnessTexture = static_cast<TextureDX11*>(material.roughnessTextures[0].get());
							RendererAPI::SetPixelShaderResource(5, roughnessTexture->GetTextureView());
						}
						else
						{
							sp.hasRoughnessTexture = 0;
							RendererAPI::SetPixelShaderResource(5, nullptr);
						}
						sp.uvTiling = material.uvTiling;
						sp.uvOffset = material.uvOffset;

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
	
	
	void RendererSystem::RenderHUD(ECS& ecs)
	{
		const WindowDimensions currentWindowDimensions = Application::Get().GetWindow()->GetRenderedZoneDimensions();
		if (currentWindowDimensions.width == 0 || currentWindowDimensions.height == 0)
		{
			return;
		}

	

		Viewport viewport;

		RendererAPI::SetViewport(viewport);

		RendererAPI::SetInputLayout(_hudVertexShader.GetInputLayout());
		RendererAPI::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		const UINT stride = sizeof(HUD_Vertex);
		const UINT offset = 0;
		RendererAPI::SetVertexBuffer(_hudVertexBuffer, stride, offset);
		RendererAPI::SetIndexBuffer(_hudIndexBuffer, 0);

		RendererAPI::EnableVertexShader(_hudVertexShader);
		RendererAPI::EnablePixelShader(_hudPixelShader);

		RendererAPI::SetVertexConstantBuffer(0, _constantBuffer.Get());

		auto& hudImages = ecs.GetDataArray<HUD_Image>();
		auto& uiButtons = ecs.GetDataArray<UIButton>();

		HUD_ShadersParams hudSp = {};
		hudSp.screenDimensions = {
			(float)currentWindowDimensions.width,
			(float)currentWindowDimensions.height,
			0.0f,
			0.0f
		};


		for (size_t i = 0; i < hudImages.size(); ++i)
		{
			const HUD_Image& hudImage = hudImages[i];

			SetRendererToFilter(hudImage.textureFilter);

			DrawImage(hudImage, hudSp);
		}


		for (const UIButton& button : uiButtons)
		{
			SetRendererToFilter(button.textureFilter);
		 	DrawImage(button, hudSp);
		}
	}

	void RendererSystem::DrawImage(const HUD_Image& hudImage, HUD_ShadersParams& hudSp)
	{
		if (hudImage.texture == nullptr || !hudImage.enabled)
		{
			return;
		}

		DirectX::XMMATRIX matScale = DirectX::XMMatrixScaling(
			hudImage.viewport.width,
			hudImage.viewport.height,
			1.0f
		);

		DirectX::XMMATRIX matTranslation = DirectX::XMMatrixTranslation(
			hudImage.viewport.x,
			hudImage.viewport.y,
			0.0f
		);

		DirectX::XMMATRIX matWorld = matScale * matTranslation;
		hudSp.world = DirectX::XMMatrixTranspose(matWorld);

		RendererAPI::UpdateSubresource(_constantBuffer.Get(), &hudSp, sizeof(HUD_ShadersParams));


		TextureDX11* diffuseTexture = static_cast<TextureDX11*>(hudImage.texture);
		RendererAPI::SetPixelShaderResource(0, diffuseTexture->GetTextureView());

		RendererAPI::DrawIndexed(_hudIndexBuffer.GetCount(), 0, 0);

		RendererAPI::SetPixelShaderResource(0, nullptr);
	}
	
	void RendererSystem::SetRendererToFilter(Material::FilterMode filterMode) {
		switch (filterMode)
		{
		case Material::FilterMode::POINT:
			RendererAPI::SetPixelSampler(0, _samplerPoint.Get());
			break;

		case Material::FilterMode::LINEAR:
			RendererAPI::SetPixelSampler(0, _samplerLinear.Get());
			break;

		case Material::FilterMode::ANISOTROPIC:
		default:
			RendererAPI::SetPixelSampler(0, _samplerAnisotropic.Get());
			break;
		}
	}
}