#include "Frost/Renderer/Model.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Renderer/TextureLibrary.h"
#include "Frost.h"

#include <DirectXMath.h>
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Frost
{
	class ModelLoading {};

	Model::Model(const std::string& filepath) : _filepath{ filepath }
	{
		Assimp::Importer aimporter;

		const aiScene* ascene = aimporter.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!ascene || ascene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ascene->mRootNode)
		{
			FT_ENGINE_ERROR("Error loading model: {}, {}", filepath, aimporter.GetErrorString());
			throw ModelLoading{};
		}

		std::filesystem::path fullPath{ filepath };
		std::string directory = fullPath.parent_path().string();

		LoadMaterials(ascene, directory);
		ProcessNode(ascene->mRootNode, ascene);

		if (_meshes.empty())
		{
			FT_ENGINE_ERROR("No meshes found in the model: {}", filepath);
			throw ModelLoading{};
		}
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			_meshes.push_back(ProcessMesh(mesh, scene));
		}

		// Iterate if there are child nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* amesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(amesh->mNumVertices);
		for (unsigned int i = 0; i < amesh->mNumVertices; ++i)
		{
			Vertex vertex;

			vertex.position = { amesh->mVertices[i].x, amesh->mVertices[i].y, amesh->mVertices[i].z };

			if (amesh->HasNormals())
			{
				vertex.normal = { amesh->mNormals[i].x, amesh->mNormals[i].y, amesh->mNormals[i].z };
			}
			else
			{
				vertex.normal = { 0.0f, 0.0f, 0.0f };
			}

			if (amesh->mTextureCoords[0])
			{
				vertex.texCoord = { amesh->mTextureCoords[0][i].x, amesh->mTextureCoords[0][i].y };
			}
			else
			{
				vertex.texCoord = { 0.0f, 0.0f };
			}


			if (amesh->HasTangentsAndBitangents())
			{
				vertex.tangent = { amesh->mTangents[i].x, amesh->mTangents[i].y, amesh->mTangents[i].z };

				vertex.bitangent = { amesh->mBitangents[i].x, amesh->mBitangents[i].y, amesh->mBitangents[i].z };
			}
			else
			{
				vertex.tangent = { 0.0f, 0.0f, 0.0f };
				vertex.bitangent = { 0.0f, 0.0f, 0.0f };
			}
			vertices.push_back(vertex);
		}

		indices.reserve(amesh->mNumFaces * 3);
		for (unsigned int i = 0; i < amesh->mNumFaces; ++i)
		{
			aiFace face = amesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		Mesh mesh(vertices, indices);
		mesh.SetMaterialIndex(amesh->mMaterialIndex);
		return mesh;
	}

	void Model::LoadMaterials(const aiScene* scene, const std::string& directory)
	{
		_materials.reserve(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* ai_material = scene->mMaterials[i];
			Material material;

			aiString name;
			ai_material->Get(AI_MATKEY_NAME, name);
			material.name = name.C_Str();

			aiColor3D color(0.0f, 0.0f, 0.0f);
			if (ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			{
				material.diffuseColor = { color.r, color.g, color.b };
			}
			else
			{
				material.diffuseColor = { 1.0f, 1.0f, 1.0f };
			}

			aiColor3D emissiveColor(0.0f, 0.0f, 0.0f);
			if (ai_material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
			{
				material.emissiveColor = { emissiveColor.r, emissiveColor.g, emissiveColor.b };
			}

			AddTexture(*ai_material, scene, directory, aiTextureType_DIFFUSE, material.diffuseTextures);
			AddTexture(*ai_material, scene, directory, aiTextureType_NORMALS, material.normalTextures);

			AddTexture(*ai_material, scene, directory, aiTextureType_EMISSIVE, material.emissiveTextures);

			if (material.emissiveTextures.empty())
			{
				AddTexture(*ai_material, scene, directory, aiTextureType_EMISSION_COLOR, material.emissiveTextures);
			}

			AddTexture(*ai_material, scene, directory, aiTextureType_AMBIENT_OCCLUSION, material.ambientOclusionTextures);

			if (material.ambientOclusionTextures.empty())
			{
				AddTexture(*ai_material, scene, directory, aiTextureType_LIGHTMAP, material.ambientOclusionTextures);
			}

			AddTexture(*ai_material, scene, directory, aiTextureType_METALNESS, material.metallicTextures);

			AddTexture(*ai_material, scene, directory, aiTextureType_DIFFUSE_ROUGHNESS, material.roughnessTextures);

			_materials.push_back(material);
		}
	}

	std::shared_ptr<Texture> Model::LoadMaterialTexture(aiMaterial* aMaterial, aiTextureType aTextureType, unsigned int index, const aiScene* aScene, const std::string& directory)
	{
		if (aMaterial->GetTextureCount(aTextureType) > 0)
		{
			aiString texturePath;
			if (aMaterial->GetTexture(aTextureType, index, &texturePath) == AI_SUCCESS)
			{
				const aiTexture* embeddedTexture = aScene->GetEmbeddedTexture(texturePath.C_Str());
				if (embeddedTexture != nullptr)
				{
					return TextureLibrary::Get().GetTexture(embeddedTexture, static_cast<TextureType>(aTextureType));
				}
				else
				{
					std::filesystem::path fullPath = std::filesystem::path(directory) / std::filesystem::path(texturePath.C_Str());
					return TextureLibrary::Get().GetTexture(fullPath.string(), static_cast<TextureType>(aTextureType));
				}
			}
		}

		return nullptr;
	}

	void Model::AddTexture(aiMaterial& ai_material, const aiScene* scene, const std::string& directory, aiTextureType textureType, std::vector<std::shared_ptr<Texture>>& emplaceHere)
	{
		unsigned int diffuseCount = ai_material.GetTextureCount(textureType);
		emplaceHere.reserve(diffuseCount);
		for (unsigned int j = 0; j < diffuseCount; j++)
		{
			std::shared_ptr<Texture> texture = LoadMaterialTexture(&ai_material, textureType, j, scene, directory);
			if (texture)
			{
				emplaceHere.push_back(texture);
			}
		}
	}
}