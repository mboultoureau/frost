#pragma once

#include "Frost/Renderer/Texture.h"

#include <string>
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace Frost
{
	struct Material
	{
		enum FilterMode {
			LINEAR,
			POINT,
			ANISOTROPIC
		};

		std::string name;
		DirectX::XMFLOAT3 diffuseColor;

		FilterMode filterMode = FilterMode::ANISOTROPIC;
		std::vector<std::shared_ptr<Texture>> diffuseTextures;
		std::vector<std::shared_ptr<Texture>> normalTextures;
		DirectX::XMFLOAT3 emissiveColor = { 0.0f, 0.0f, 0.0f };
		std::vector<std::shared_ptr<Texture>> emissiveTextures;
		std::vector<std::shared_ptr<Texture>> ambientOclusionTextures;
		std::vector<std::shared_ptr<Texture>> metallicTextures;
		std::vector<std::shared_ptr<Texture>> roughnessTextures;
		float roughnessValue = 0.5f;
		DirectX::XMFLOAT2 uvTiling = { 1,1 };
		DirectX::XMFLOAT2 uvOffset = { 0,0 };
	};
}


