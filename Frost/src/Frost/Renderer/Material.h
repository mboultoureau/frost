#pragma once

#include "Frost/Asset/Texture.h"
#include "Frost/Renderer/Shader.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Utils/Math/Vector.h"

#include <memory>
#include <string>
#include <vector>

namespace Frost
{
    struct Material
    {
        enum class FilterMode
        {
            POINT,
            LINEAR,
            ANISOTROPIC
        };
        enum class Topology
        {
            TRIANGLES,
            PATCH_3
        };

        // Name (for debugging)
        std::string name = "DefaultMaterial";

        // Colors
        Math::Color4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
        Math::Color4 emissiveColor = { 0.0f, 0.0f, 0.0f, 1.0f };
        float metalness = 0.0f;
        float roughness = 0.5f;
        float ao = 1.0f;

        // Textures
        FilterMode filter = FilterMode::ANISOTROPIC;
        std::vector<std::shared_ptr<Texture>> albedoTextures;
        std::vector<std::shared_ptr<Texture>> normalTextures;
        std::vector<std::shared_ptr<Texture>> metallicTextures;
        std::vector<std::shared_ptr<Texture>> roughnessTextures;
        std::vector<std::shared_ptr<Texture>> aoTextures;
        std::vector<std::shared_ptr<Texture>> emissiveTextures;
        std::vector<uint8_t> parameters;

        // UV transformations
        Math::Vector2 uvTiling = { 1, 1 };
        Math::Vector2 uvOffset = { 0, 0 };

        // Custom shaders
        std::shared_ptr<Shader> customVertexShader;
        std::shared_ptr<Shader> customPixelShader;
        std::shared_ptr<Shader> geometryShader;
        std::shared_ptr<Shader> hullShader;
        std::shared_ptr<Shader> domainShader;

        // Topology
        Topology topology = Topology::TRIANGLES;

        // If present, the render target of the camera will be used as albedo
        // texture
        GameObject::Id cameraRef;
    };
} // namespace Frost