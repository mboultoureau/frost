#pragma once

#include "Frost/Renderer/Mesh.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Texture.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;

namespace Frost
{
	class Model
	{
	public:
		Model(const std::string& filepath);
		const std::string& GetFilepath() const { return _filepath; }
		const std::vector<Mesh>& GetMeshes() const { return _meshes; }
		const std::vector<Material>& GetMaterials() const { return _materials; }
		Model() = default;

	protected:
		std::string _filepath;
		std::vector<Mesh> _meshes;
		std::vector<Material> _materials;

		void ProcessNode(aiNode* aNode, const aiScene* aScene);
		Mesh ProcessMesh(aiMesh* aMesh, const aiScene* aScene);
		void LoadMaterials(const aiScene* aScene, const std::string& directory);
		std::shared_ptr<Texture> LoadMaterialTexture(aiMaterial* aMaterial, aiTextureType aTextureType, unsigned int index, const aiScene* aScene, const std::string& directory);
		void AddTexture(aiMaterial& material, const aiScene* scene, const std::string& directory, aiTextureType textureType, std::vector<std::shared_ptr<Texture>>& emplaceHere);
	};
}