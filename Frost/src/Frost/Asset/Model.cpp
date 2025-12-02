#include "Frost/Asset/Model.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Renderer/DX11/TextureDX11.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Utils/File/MemoryMappedFile.h"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

#undef max

namespace Frost
{
    Model::Model(const std::string& filepath) : _filepath(filepath)
    {
        FT_ENGINE_INFO("Loading model from: {}", filepath);

        LoadCPU(filepath);
        UploadGPU();
    }

    void Model::LoadCPU(const std::string& filepath)
    {
        _filepath = filepath;
        _directory = std::filesystem::path(filepath).parent_path().string();

        FT_ENGINE_INFO("Async Loading model: {}", filepath);

        Assimp::Importer importer;
        unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                             aiProcess_CalcTangentSpace | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder |
                             aiProcess_PreTransformVertices;

        const aiScene* scene = importer.ReadFile(filepath, flags);

        // If you load glb2 ou gltf2 an exception is throw but the model is loaded
        // See: https://github.com/assimp/assimp/issues/2778
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            FT_ENGINE_ERROR("Assimp Error: {}", importer.GetErrorString());
            SetStatus(AssetStatus::Failed);
            return;
        }

        LoadMaterials(scene);
        ProcessNode(scene->mRootNode, scene);

        FT_ENGINE_INFO("Model loaded successfully: {} meshes, {} materials", _meshes.size(), _materials.size());
    }

    void Model::UploadGPU()
    {
        if (GetStatus() == AssetStatus::Failed)
        {
            return;
        }

        _meshes.reserve(_cpuMeshes.size());
        for (auto& cpuMesh : _cpuMeshes)
        {
            auto vertexData = std::as_bytes(std::span(cpuMesh.vertices));
            _meshes.emplace_back(vertexData, static_cast<uint32_t>(sizeof(Vertex)), cpuMesh.indices);
            _meshes.back().SetMaterialIndex(cpuMesh.materialIndex);
        }

        _cpuMeshes.clear();

        FT_ENGINE_INFO("Model uploaded to GPU: {}", _filepath);
        SetStatus(AssetStatus::Loaded);
    }

    void Model::ProcessNode(aiNode* aNode, const aiScene* aScene)
    {
        // Process all meshes of the current node
        for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
        {
            aiMesh* mesh = aScene->mMeshes[aNode->mMeshes[i]];
            ProcessMesh(mesh, aScene);
        }

        // Repeat the process for all child nodes
        for (unsigned int i = 0; i < aNode->mNumChildren; i++)
        {
            ProcessNode(aNode->mChildren[i], aScene);
        }
    }

    void Model::ProcessMesh(aiMesh* aMesh, const aiScene* aScene)
    {
        std::vector<Vertex> vertices;
        vertices.reserve(aMesh->mNumVertices);

        for (unsigned int i = 0; i < aMesh->mNumVertices; i++)
        {
            Vertex vertex{};
            vertex.position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };

            if (aMesh->HasNormals())
            {
                vertex.normal = { aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z };
            }

            if (aMesh->mTextureCoords[0])
            {
                vertex.texCoord = { aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y };
            }

            if (aMesh->HasTangentsAndBitangents())
            {
                vertex.tangent = { aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z, 1.0f };
            }

            vertices.push_back(vertex);
        }

        std::vector<uint32_t> indices;
        indices.reserve(static_cast<size_t>(aMesh->mNumFaces) * 3);
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++)
        {
            aiFace face = aMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        auto vertexData = std::as_bytes(std::span(vertices));

        _cpuMeshes.emplace_back(std::move(vertices), std::move(indices), aMesh->mMaterialIndex);
    }

    void Model::LoadMaterials(const aiScene* scene)
    {
        if (!scene->HasMaterials())
            return;

        _materials.reserve(scene->mNumMaterials);

        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            aiMaterial* ai_material = scene->mMaterials[i];
            Material material;

            LoadMaterialProperties(ai_material, material);

            // Load all texture types
            LoadMaterialTextures(scene, ai_material, aiTextureType_DIFFUSE, material.albedoTextures);
            LoadMaterialTextures(scene, ai_material, aiTextureType_NORMALS, material.normalTextures);
            LoadMaterialTextures(scene, ai_material, aiTextureType_METALNESS, material.metallicTextures);
            LoadMaterialTextures(scene, ai_material, aiTextureType_DIFFUSE_ROUGHNESS, material.roughnessTextures);
            LoadMaterialTextures(scene, ai_material, aiTextureType_AMBIENT_OCCLUSION, material.aoTextures);
            LoadMaterialTextures(scene, ai_material, aiTextureType_EMISSIVE, material.emissiveTextures);

            _materials.emplace_back(material);
        }
    }

    void Model::LoadMaterialProperties(const aiMaterial* ai_material, Material& material)
    {
        aiString name;
        if (ai_material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
        {
            material.name = name.C_Str();
        }

        aiColor3D diffuseColor;
        if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS)
        {
            material.albedo = { diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f };
        }

        aiColor3D emissiveColor;
        if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
        {
            material.emissiveColor = { emissiveColor.r, emissiveColor.g, emissiveColor.b, 1.0f };
        }

        float metallicFactor;
        if (ai_material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS)
        {
            material.metalness = metallicFactor;
        }

        float roughnessFactor;
        if (ai_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS)
        {
            material.roughness = roughnessFactor;
        }

        aiUVTransform uvTransform;
        if (ai_material->Get(AI_MATKEY_UVTRANSFORM(aiTextureType_DIFFUSE, 0), uvTransform) == AI_SUCCESS)
        {
            material.uvTiling = { uvTransform.mScaling.x, uvTransform.mScaling.y };
            // Note: Assimp's mTranslation can sometimes require adjustments
            // depending on the model's origin.
            material.uvOffset = { uvTransform.mTranslation.x, uvTransform.mTranslation.y };
        }
    }

    void Model::LoadMaterialTextures(const aiScene* scene,
                                     const aiMaterial* ai_material,
                                     aiTextureType type,
                                     std::vector<std::shared_ptr<Texture>>& outTextures)
    {
        unsigned int textureCount = ai_material->GetTextureCount(type);
        if (textureCount == 0)
            return;

        outTextures.reserve(textureCount);

        for (unsigned int i = 0; i < textureCount; i++)
        {
            aiString str;
            if (ai_material->GetTexture(type, i, &str) != AI_SUCCESS)
                continue;

            std::string textureIdentifier = str.C_Str();
            TextureConfig config;
            std::string assetId;

            switch (type)
            {
                case aiTextureType_DIFFUSE:
                    config.textureType = TextureType::DIFFUSE;
                    break;
                case aiTextureType_NORMALS:
                    config.textureType = TextureType::NORMALS;
                    break;
                case aiTextureType_METALNESS:
                    config.textureType = TextureType::METALNESS;
                    break;
                case aiTextureType_DIFFUSE_ROUGHNESS:
                    config.textureType = TextureType::DIFFUSE_ROUGHNESS;
                    break;
                case aiTextureType_AMBIENT_OCCLUSION:
                    config.textureType = TextureType::AMBIENT_OCCLUSION;
                    break;
                case aiTextureType_EMISSIVE:
                    config.textureType = TextureType::EMISSIVE;
                    break;
                default:
                    config.textureType = TextureType::UNKNOWN;
                    break;
            }

            // Check if texture is embedded
            if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(textureIdentifier.c_str()))
            {
                config.debugName = textureIdentifier;
                assetId = _filepath + "?" + textureIdentifier;

                if (embeddedTexture->mHeight == 0)
                {
                    size_t size = embeddedTexture->mWidth;
                    config.fileData.assign(reinterpret_cast<const uint8_t*>(embeddedTexture->pcData),
                                           reinterpret_cast<const uint8_t*>(embeddedTexture->pcData) + size);
                    config.width = 0;
                    config.height = 0;
                    config.format = Format::RGBA8_UNORM;
                    config.isCompressed = true;
                    config.channels = 0;
                }
                else
                {
                    config.width = embeddedTexture->mWidth;
                    config.height = embeddedTexture->mHeight;
                    config.channels = 4;
                    config.format = Format::RGBA8_UNORM;

                    size_t numPixels = embeddedTexture->mWidth * embeddedTexture->mHeight;
                    config.fileData.resize(numPixels * 4);

                    const aiTexel* src = embeddedTexture->pcData;
                    uint8_t* dst = config.fileData.data();

                    for (size_t p = 0; p < numPixels; ++p)
                    {
                        dst[p * 4 + 0] = src[p].r;
                        dst[p * 4 + 1] = src[p].g;
                        dst[p * 4 + 2] = src[p].b;
                        dst[p * 4 + 3] = src[p].a;
                    }
                }
            }
            else
            {
                std::filesystem::path texturePath =
                    std::filesystem::path(_directory) / std::filesystem::path(textureIdentifier);
                assetId = texturePath.string();
                config.path = assetId;
            }

            outTextures.push_back(AssetManager::LoadAsset(assetId, config));
        }
    }

    BoundingBox Model::GetBoundingBox() const
    {
        Frost::BoundingBox totalBounds = { { FLT_MAX, FLT_MAX, FLT_MAX }, { -FLT_MAX, -FLT_MAX, -FLT_MAX } };

        for (const auto& mesh : _meshes)
        {
            const BoundingBox& meshBounds = mesh.GetBoundingBox();
            totalBounds.min.x = std::min(totalBounds.min.x, meshBounds.min.x);
            totalBounds.min.y = std::min(totalBounds.min.y, meshBounds.min.y);
            totalBounds.min.z = std::min(totalBounds.min.z, meshBounds.min.z);
            totalBounds.max.x = std::max(totalBounds.max.x, meshBounds.max.x);
            totalBounds.max.y = std::max(totalBounds.max.y, meshBounds.max.y);
            totalBounds.max.z = std::max(totalBounds.max.z, meshBounds.max.z);
        }

        return totalBounds;
    }
} // namespace Frost