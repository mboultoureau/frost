#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Mesh.h"
#include <string>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace Frost
{
    class Renderer;

    class Model : public Asset
    {
    public:
        Model(const std::string& filepath);
        Model() = default;

        const std::string& GetFilepath() const { return _filepath; }
        const std::vector<Mesh>& GetMeshes() const { return _meshes; }
        const std::vector<Material>& GetMaterials() const { return _materials; }
        std::vector<Mesh>& GetMeshes() { return _meshes; }
        std::vector<Material>& GetMaterials() { return _materials; }

        void AddMesh(Mesh&& mesh) { _meshes.emplace_back(std::move(mesh)); }
        void AddMaterial(Material&& mat) { _materials.emplace_back(std::move(mat)); }

        bool HasMaterials() const { return !_materials.empty(); }
        bool HasMeshes() const { return !_meshes.empty(); }
        BoundingBox GetBoundingBox() const;

    private:
        void ProcessNode(Renderer* renderer, aiNode* aNode, const aiScene* aScene);
        void ProcessMesh(Renderer* renderer, aiMesh* aMesh, const aiScene* aScene);
        void LoadMaterials(const aiScene* aScene);
        void LoadMaterialProperties(const aiMaterial* ai_material, Material& material);
        void LoadMaterialTextures(const aiScene* scene,
                                  const aiMaterial* ai_material,
                                  aiTextureType type,
                                  std::vector<std::shared_ptr<Texture>>& outTextures);

    protected:
        std::string _filepath;
        std::string _directory;
        std::vector<Mesh> _meshes;
        std::vector<Material> _materials;
    };
} // namespace Frost