#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Utils/Math/Vector.h"
#include "Frost/Scene/ECS/GameObject.h"

#include <string>
#include <memory>
#include <vector>

namespace Frost
{
	struct Material
	{
		enum class FilterMode { POINT, LINEAR, ANISOTROPIC };

		std::string name = "DefaultMaterial";
		Math::Color4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
		Math::Color4 emissiveColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		float metalness = 0.0f;
		float roughness = 0.5f;
		float ao = 1.0f;
		
		FilterMode filter = FilterMode::ANISOTROPIC;
		std::vector<std::shared_ptr<Texture>> albedoTextures;
		std::vector<std::shared_ptr<Texture>> normalTextures;
		std::vector<std::shared_ptr<Texture>> metallicTextures;
		std::vector<std::shared_ptr<Texture>> roughnessTextures;
		std::vector<std::shared_ptr<Texture>> aoTextures;
		std::vector<std::shared_ptr<Texture>> emissiveTextures;
		
		Math::Vector2 uvTiling = { 1,1 };
		Math::Vector2 uvOffset = { 0,0 };

		// If present, the render target of the camera will be used as albedo texture
		GameObject::Id cameraRef;
	};
}