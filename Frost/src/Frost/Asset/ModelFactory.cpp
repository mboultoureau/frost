#include "Frost/Asset/ModelFactory.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Utils/Math/Vector.h"

using namespace Frost::Math;

// windows.....
#undef min;
#undef max;

namespace Frost
{
    // Helper function to add a mesh
    static void AddMeshToModel(std::shared_ptr<Model>& model,
                               std::vector<Vertex>& vertices,
                               std::vector<uint32_t>& indices)
    {
        // Default Material
        if (model->GetMaterials().empty())
        {
            Material defaultMat;
            defaultMat.name = "DefaultPrimitiveMat";
            defaultMat.albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
            defaultMat.roughness = 0.5f;
            defaultMat.metalness = 0.0f;
            model->AddMaterial(std::move(defaultMat));
        }

        auto vertexData = std::as_bytes(std::span(vertices));

        Mesh mesh(vertexData, static_cast<uint32_t>(sizeof(Vertex)), indices);
        mesh.SetMaterialIndex(0);

        model->AddMesh(std::move(mesh));
    }

    std::shared_ptr<Model> ModelFactory::CreateFromFile(const std::string& filepath)
    {
        return AssetManager::LoadAsset<Model>(filepath, filepath);
    }

    std::shared_ptr<Model> ModelFactory::CreateFromHeightMap(HeightMapConfig config)
    {
        return std::make_shared<HeightMapModel>(config);
    }

    std::shared_ptr<Model> ModelFactory::CreateCube(float size)
    {
        auto model = std::make_shared<Model>();
        float half = size * 0.5f;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        vertices.reserve(24);
        indices.reserve(36);

        auto AddFace = [&](const Vector3& normal, const Vector3& tangent, const Vector3& up)
        {
            Vector3 right = tangent;

            // Coins : BL, BR, TR, TL
            Vector3 c0 = normal * half - right * half - up * half;
            Vector3 c1 = normal * half + right * half - up * half;
            Vector3 c2 = normal * half + right * half + up * half;
            Vector3 c3 = normal * half - right * half + up * half;

            uint32_t baseIdx = static_cast<uint32_t>(vertices.size());

            // Vertex structure: Pos, Normal, TexCoord, Tangent
            vertices.push_back({ c0, normal, { 0.0f, 1.0f }, { tangent.x, tangent.y, tangent.z, 1.0f } });
            vertices.push_back({ c1, normal, { 1.0f, 1.0f }, { tangent.x, tangent.y, tangent.z, 1.0f } });
            vertices.push_back({ c2, normal, { 1.0f, 0.0f }, { tangent.x, tangent.y, tangent.z, 1.0f } });
            vertices.push_back({ c3, normal, { 0.0f, 0.0f }, { tangent.x, tangent.y, tangent.z, 1.0f } });

            // Indices
            indices.push_back(baseIdx + 0);
            indices.push_back(baseIdx + 1);
            indices.push_back(baseIdx + 2);
            indices.push_back(baseIdx + 2);
            indices.push_back(baseIdx + 3);
            indices.push_back(baseIdx + 0);
        };

        // Front (+Z)
        AddFace({ 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 });
        // Back (-Z)
        AddFace({ 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 });
        // Top (+Y)
        AddFace({ 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 });
        // Bottom (-Y)
        AddFace({ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 });
        // Right (+X)
        AddFace({ 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 });
        // Left (-X)
        AddFace({ -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 });

        AddMeshToModel(model, vertices, indices);
        return model;
    }
    std::shared_ptr<Model> ModelFactory::CreateCubeWithBevel(float size, float bevelSize)
    {
        auto model = std::make_shared<Model>();
        float half = size * 0.5f;

        bevelSize = std::min(bevelSize, half * 0.49f);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        auto AddBeveledFace = [&](Vector3 normal, Vector3 tangent, Vector3 up)
        {
            using namespace JPH;
            Vector3 right = tangent;

            float o = half;
            float i = half - bevelSize;

            float coords[4] = { -o, -i, i, o };

            uint32_t baseIdx = static_cast<uint32_t>(vertices.size());

            for (int iy = 0; iy < 4; iy++)
            {
                for (int ix = 0; ix < 4; ix++)
                {
                    Vector3 pos = normal * o + right * coords[ix] + up * coords[iy];

                    Vector3 faceN = normal;
                    Vector3 edgeXN = right;
                    Vector3 edgeYN = up;

                    float bx = (ix == 1 || ix == 2) ? 1.0f : 0.0f;
                    float by = (iy == 1 || iy == 2) ? 1.0f : 0.0f;

                    JPH::Vec3 _n = vector_cast<JPH::Vec3>(faceN);
                    _n += vector_cast<Vec3>(edgeXN) * bx * 0.4f;
                    _n += vector_cast<Vec3>(edgeYN) * by * 0.4f;
                    auto n = vector_cast<Vector3>(_n.Normalized());

                    float u = float(ix) / 3.0f;
                    float v = 1.0f - float(iy) / 3.0f;

                    vertices.push_back({ pos, n, { u, v }, { tangent.x, tangent.y, tangent.z, 1.0f } });
                }
            }

            for (int y = 0; y < 3; y++)
            {
                for (int x = 0; x < 3; x++)
                {
                    uint32_t i0 = baseIdx + y * 4 + x;
                    uint32_t i1 = baseIdx + y * 4 + x + 1;
                    uint32_t i2 = baseIdx + (y + 1) * 4 + x;
                    uint32_t i3 = baseIdx + (y + 1) * 4 + x + 1;

                    indices.push_back(i0);
                    indices.push_back(i1);
                    indices.push_back(i3);
                    indices.push_back(i0);
                    indices.push_back(i3);
                    indices.push_back(i2);
                }
            }
        };

        AddBeveledFace({ 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 });   // Front
        AddBeveledFace({ 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 }); // Back
        AddBeveledFace({ 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 });  // Top
        AddBeveledFace({ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 });  // Bottom
        AddBeveledFace({ 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 });  // Right
        AddBeveledFace({ -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 });  // Left

        AddMeshToModel(model, vertices, indices);
        return model;
    }

    std::shared_ptr<Model> ModelFactory::CreateCubeWithPrecision(float size, const Vector3& nbVertices)
    {
        auto model = std::make_shared<Model>();
        float half = size * 0.5f;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        auto AddFace = [&](const Vector3& normal, const Vector3& tangent, const Vector3& up, int nx, int ny)
        {
            Vector3 right = tangent;

            // Corners of the face
            Vector3 c0 = normal * half - right * half - up * half;
            Vector3 c1 = normal * half + right * half - up * half;
            Vector3 c2 = normal * half + right * half + up * half;
            Vector3 c3 = normal * half - right * half + up * half;

            uint32_t baseIndex = (uint32_t)vertices.size();

            for (int iy = 0; iy < ny; iy++)
            {
                float v = float(iy) / float(ny - 1);
                Vector3 edgeL = c0 + (c3 - c0) * v;
                Vector3 edgeR = c1 + (c2 - c1) * v;

                for (int ix = 0; ix < nx; ix++)
                {
                    float u = float(ix) / float(nx - 1);
                    Vector3 pos = edgeL + (edgeR - edgeL) * u;

                    vertices.push_back({ pos, normal, { u, 1.0f - v }, { tangent.x, tangent.y, tangent.z, 1.0f } });
                }
            }

            // Indices
            for (int iy = 0; iy < ny - 1; iy++)
            {
                for (int ix = 0; ix < nx - 1; ix++)
                {
                    uint32_t i0 = baseIndex + ix + iy * nx;
                    uint32_t i1 = baseIndex + (ix + 1) + iy * nx;
                    uint32_t i2 = baseIndex + (ix + 1) + (iy + 1) * nx;
                    uint32_t i3 = baseIndex + ix + (iy + 1) * nx;

                    indices.push_back(i0);
                    indices.push_back(i1);
                    indices.push_back(i2);

                    indices.push_back(i2);
                    indices.push_back(i3);
                    indices.push_back(i0);
                }
            }
        };

        int nx = (int)nbVertices.x;
        int ny = (int)nbVertices.y;
        int nz = (int)nbVertices.z;

        // --- 6 faces avec subdivision correcte ---

        // Front  (X × Y)
        AddFace({ 0, 0, 1 }, { 1, 0, 0 }, { 0, 1, 0 }, nx, ny);

        // Back   (X × Y)
        AddFace({ 0, 0, -1 }, { -1, 0, 0 }, { 0, 1, 0 }, nx, ny);

        // Top    (X × Z)
        AddFace({ 0, 1, 0 }, { 1, 0, 0 }, { 0, 0, -1 }, nx, nz);

        // Bottom (X × Z)
        AddFace({ 0, -1, 0 }, { 1, 0, 0 }, { 0, 0, 1 }, nx, nz);

        // Right  (Z × Y)
        AddFace({ 1, 0, 0 }, { 0, 0, -1 }, { 0, 1, 0 }, nz, ny);

        // Left   (Z × Y)
        AddFace({ -1, 0, 0 }, { 0, 0, 1 }, { 0, 1, 0 }, nz, ny);

        AddMeshToModel(model, vertices, indices);
        return model;
    }

    std::shared_ptr<Model> ModelFactory::CreatePlane(float width, float depth)
    {
        auto model = std::make_shared<Model>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float hw = width * 0.5f;
        float hd = depth * 0.5f;

        Vector3 n = { 0.0f, 1.0f, 0.0f };
        Vector3 t = { 1.0f, 0.0f, 0.0f };

        vertices.push_back({ { -hw, 0.0f, -hd }, n, { 0.0f, 1.0f }, { t.x, t.y, t.z, 1.0f } }); // BL
        vertices.push_back({ { hw, 0.0f, -hd }, n, { 1.0f, 1.0f }, { t.x, t.y, t.z, 1.0f } });  // BR
        vertices.push_back({ { hw, 0.0f, hd }, n, { 1.0f, 0.0f }, { t.x, t.y, t.z, 1.0f } });   // TR
        vertices.push_back({ { -hw, 0.0f, hd }, n, { 0.0f, 0.0f }, { t.x, t.y, t.z, 1.0f } });  // TL

        indices = { 0, 1, 2, 2, 3, 0 };

        AddMeshToModel(model, vertices, indices);
        return model;
    }

    std::shared_ptr<Model> ModelFactory::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
    {
        auto model = std::make_shared<Model>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        Vertex topVertex;
        topVertex.position = { 0, radius, 0 };
        topVertex.normal = { 0, 1, 0 };
        topVertex.texCoord = { 0, 0 };
        topVertex.tangent = { 1, 0, 0, 1 };

        Vertex bottomVertex;
        bottomVertex.position = { 0, -radius, 0 };
        bottomVertex.normal = { 0, -1, 0 };
        bottomVertex.texCoord = { 0, 1 };
        bottomVertex.tangent = { 1, 0, 0, 1 };

        vertices.push_back(topVertex);

        float phiStep = std::numbers::pi_v<float> / stackCount;
        float thetaStep = 2.0f * std::numbers::pi_v<float> / sliceCount;

        for (uint32_t i = 1; i <= stackCount - 1; ++i)
        {
            float phi = i * phiStep;

            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;
                float sinTheta = std::sin(theta);
                float cosTheta = std::cos(theta);

                float x = radius * sinPhi * cosTheta;
                float y = radius * cosPhi;
                float z = radius * sinPhi * sinTheta;

                Vector3 pos = { x, y, z };
                Vector3 normal = { sinPhi * cosTheta, cosPhi, sinPhi * sinTheta };
                Vector3 tangent = { -sinTheta, 0.0f, cosTheta };

                // UV
                float u = (float)j / sliceCount;
                float v = (float)i / stackCount;

                vertices.push_back({ pos, normal, { u, v }, { tangent.x, tangent.y, tangent.z, 1.0f } });
            }
        }

        vertices.push_back(bottomVertex);

        uint32_t northPoleIndex = 0;
        uint32_t southPoleIndex = (uint32_t)vertices.size() - 1;
        uint32_t ringVertexCount = sliceCount + 1;

        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            indices.push_back(northPoleIndex);
            indices.push_back(i + 2);
            indices.push_back(i + 1);
        }

        uint32_t baseIndex = 1;
        for (uint32_t i = 0; i < stackCount - 2; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                indices.push_back(baseIndex + i * ringVertexCount + j);
                indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

                indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
                indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
            }
        }

        uint32_t startParams = baseIndex + (stackCount - 2) * ringVertexCount;
        for (uint32_t i = 0; i < sliceCount; ++i)
        {
            indices.push_back(southPoleIndex);
            indices.push_back(startParams + i);
            indices.push_back(startParams + i + 1);
        }

        AddMeshToModel(model, vertices, indices);
        return model;
    }
} // namespace Frost