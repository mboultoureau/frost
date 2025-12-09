#pragma once

#include "Frost/Asset/Asset.h"
#include "Frost/Core/Core.h"
#include "Frost/Renderer/Material.h"
#include "Frost/Renderer/Mesh.h"

#include <string>
#include <vector>
#include <atomic>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace Frost
{
    class Renderer;

    class FROST_API Model : public Asset
    {
    public:
        Model() = default;
        Model(const std::string& filepath);

        void LoadCPU(const std::string& filepath);
        void UploadGPU();

        const std::string& GetFilepath() const { return _filepath; }
        const std::vector<Mesh>& GetMeshes() const { return _meshes; }
        const std::vector<Material>& GetMaterials() const { return _materials; }
        std::vector<Mesh>& GetMeshes() { return _meshes; }
        std::vector<Material>& GetMaterials() { return _materials; }

        void AddMesh(Mesh&& mesh) { _meshes.emplace_back(std::move(mesh)); }
        void AddMaterial(Material&& mat) { _materials.emplace_back(std::move(mat)); }
        void SetMeshes(std::vector<Mesh>&& meshes) { _meshes = std::move(meshes); }

        bool HasMaterials() const { return IsLoaded() && !_materials.empty(); }
        bool HasMeshes() const { return IsLoaded() && !_meshes.empty(); }
        BoundingBox GetBoundingBox() const;

    private:
        void ProcessNode(aiNode* aNode, const aiScene* aScene);
        void ProcessMesh(aiMesh* aMesh, const aiScene* aScene);
        void LoadMaterials(const aiScene* aScene);
        void LoadMaterialProperties(const aiMaterial* ai_material, Material& material);
        void LoadMaterialTextures(const aiScene* scene,
                                  const aiMaterial* ai_material,
                                  aiTextureType type,
                                  std::vector<std::shared_ptr<Texture>>& outTextures);

    protected:
        struct CpuMeshData
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            uint32_t materialIndex;
        };

        std::string _filepath;
        std::string _directory;
        std::vector<Mesh> _meshes;
        std::vector<Material> _materials;
        std::vector<CpuMeshData> _cpuMeshes;
    };
} // namespace Frost