#include "Mesh.h"

#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Frost/Renderer/Vertex.h"

namespace Frost
{
	class MeshLoading{};

	Mesh::Mesh(const std::string& filepath) : _filepath{ filepath }
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			// Display error message in MessageBox
			MessageBoxA(NULL, importer.GetErrorString(), "Error loading model", MB_OK | MB_ICONERROR);
			throw MeshLoading{};
		}

		if (scene->mNumMeshes == 0)
		{
			MessageBoxA(NULL, "No meshes found in the model.", "Error loading model", MB_OK | MB_ICONERROR);
			throw MeshLoading{};
		}

		const aiMesh* mesh = scene->mMeshes[0];
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		vertices.reserve(mesh->mNumVertices);
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex vertex;
			// Positions
			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;
			// Normals
			if (mesh->HasNormals())
			{
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;
			}
			else
			{
				vertex.normal = { 0.0f, 0.0f, 0.0f };
			}
			// Texture Coordinates
			if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
			{
				vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texCoord = { 0.0f, 0.0f };
			}
			vertices.push_back(vertex);
		}

		indices.reserve(mesh->mNumFaces * 3);
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		_vertexBuffer.Create(vertices.data(), static_cast<UINT>(vertices.size() * sizeof(Vertex)));
		_indexBuffer.Create(indices.data(), static_cast<UINT>(indices.size() * sizeof(uint32_t)), indices.size());
	}
}