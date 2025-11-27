#include "Frost/Asset/Model.h"
#include "Frost/Renderer/RendererAPI.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Debugging/Assert.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Renderer/DX11/TextureDX11.h"

#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

namespace Frost
{
	Model::Model(const std::string& filepath)
		: _filepath(filepath)
	{
		FT_ENGINE_INFO("Loading model from: {}", filepath);

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filepath,
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_CalcTangentSpace);


		// If you load glb2 ou gltf2 an exception is throw but the model is loaded
		// See: https://github.com/assimp/assimp/issues/2778
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			FT_ENGINE_ERROR("Assimp Error: {}", importer.GetErrorString());
			return;
		}

		_directory = std::filesystem::path(filepath).parent_path().string();
		Renderer* renderer = RendererAPI::GetRenderer();

		LoadMaterials(scene);
		ProcessNode(renderer, scene->mRootNode, scene);

		FT_ENGINE_INFO("Model loaded successfully: {} meshes, {} materials", _meshes.size(), _materials.size());
	}

	void Model::ProcessNode(Renderer* renderer, aiNode* aNode, const aiScene* aScene)
	{
		// Process all meshes of the current node
		for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		{
			aiMesh* mesh = aScene->mMeshes[aNode->mMeshes[i]];
			ProcessMesh(renderer, mesh, aScene);
		}

		// Repeat the process for all child nodes
		for (unsigned int i = 0; i < aNode->mNumChildren; i++)
		{
			ProcessNode(renderer, aNode->mChildren[i], aScene);
		}
	}

	void Model::ProcessMesh(Renderer* renderer, aiMesh* aMesh, const aiScene* aScene)
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

		_meshes.emplace_back(vertexData, static_cast<uint32_t>(sizeof(Vertex)), indices);
		_meshes.back().SetMaterialIndex(aMesh->mMaterialIndex);
	}

	void Model::LoadMaterials(const aiScene* scene)
	{
		if (!scene->HasMaterials()) return;

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
			// Note: Assimp's mTranslation can sometimes require adjustments depending on the model's origin.
			material.uvOffset = { uvTransform.mTranslation.x, uvTransform.mTranslation.y };
		}
	}

	void Model::LoadMaterialTextures(const aiScene* scene, const aiMaterial* ai_material, aiTextureType type, std::vector<std::shared_ptr<Texture>>& outTextures)
	{
		unsigned int textureCount = ai_material->GetTextureCount(type);
		if (textureCount == 0) return;

		outTextures.reserve(textureCount);

		for (unsigned int i = 0; i < textureCount; i++)
		{
			aiString str;
			if (ai_material->GetTexture(type, i, &str) != AI_SUCCESS) continue;

			std::string textureIdentifier = str.C_Str();
			TextureConfig config;
			std::string assetId;

			// Determine texture type for config
			switch (type)
			{
			case aiTextureType_DIFFUSE:   config.textureType = TextureType::DIFFUSE; break;
			case aiTextureType_NORMALS:   config.textureType = TextureType::NORMALS; break;
			case aiTextureType_METALNESS: config.textureType = TextureType::METALNESS; break;
			case aiTextureType_DIFFUSE_ROUGHNESS: config.textureType = TextureType::DIFFUSE_ROUGHNESS; break;
			case aiTextureType_AMBIENT_OCCLUSION: config.textureType = TextureType::AMBIENT_OCCLUSION; break;
			case aiTextureType_EMISSIVE:  config.textureType = TextureType::EMISSIVE; break;
			default:                      config.textureType = TextureType::UNKNOWN; break;
			}

			// Check if texture is embedded
			if (const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(textureIdentifier.c_str()))
			{
				config.aTexture = const_cast<aiTexture*>(embeddedTexture);
				config.debugName = textureIdentifier;
				assetId = _filepath + "?" + textureIdentifier;
			}
			else
			{
				std::filesystem::path texturePath = std::filesystem::path(_directory) / std::filesystem::path(textureIdentifier);
				assetId = texturePath.string();
				config.path = assetId;
			}

			outTextures.push_back(AssetManager::LoadAsset<TextureDX11>(assetId, config));
		}
	}
}