#include "Frost/Asset/ModelFactory.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Utils/Math/Vector.h"

using namespace Frost::Math;

// windows.....
#undef min
#undef max

namespace Frost
{
    float GetHeightFromPixel(const std::vector<uint8_t>& data,
                             uint32_t imgW,
                             uint32_t imgH,
                             uint32_t channels,
                             float u,
                             float v)
    {
        if (data.empty())
            return 0.0f;

        u = std::clamp(u, 0.0f, 1.0f);
        v = std::clamp(v, 0.0f, 1.0f);

        uint32_t x = static_cast<uint32_t>(u * (imgW - 1));
        uint32_t y = static_cast<uint32_t>(v * (imgH - 1));

        size_t index = (y * imgW + x) * channels;
        if (index >= data.size())
            return 0.0f;

        return static_cast<float>(data[index]) / 255.0f;
    }

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
        return AssetManager::LoadAsset<Model>(filepath);
    }

    std::shared_ptr<Model> ModelFactory::CreateFromHeightMap(const Component::MeshSourceHeightMap& config)
    {
        TextureConfig texConfig;
        texConfig.loadImmediately = true;
        texConfig.path = config.texturePath.generic_string();
        texConfig.textureType = TextureType::HEIGHTMAP;
        auto texture = Texture::Create(texConfig);

        std::vector<uint8_t> emptyData;
        const auto& pixels = texture ? texture->GetData() : emptyData;
        uint32_t imgW = texture ? texture->GetWidth() : 0;
        uint32_t imgH = texture ? texture->GetHeight() : 0;
        uint32_t channels = texture ? texture->GetChannels() : 0;
        bool hasData = !pixels.empty() && imgW > 0 && imgH > 0;

        auto model = std::make_shared<Model>();

        uint32_t chunksX = (config.segmentsWidth + config.chunkSize - 1) / config.chunkSize;
        uint32_t chunksZ = (config.segmentsDepth + config.chunkSize - 1) / config.chunkSize;

        float totalWidth = config.width;
        float totalDepth = config.depth;
        float halfW = totalWidth * 0.5f;
        float halfD = totalDepth * 0.5f;
        float heightRange = config.maxHeight - config.minHeight;

        float stepX = totalWidth / static_cast<float>(config.segmentsWidth);
        float stepZ = totalDepth / static_cast<float>(config.segmentsDepth);

        // std::vector<std::future<void>> futures;
        // std::mutex modelMutex;

        for (uint32_t cz = 0; cz < chunksZ; ++cz)
        {
            for (uint32_t cx = 0; cx < chunksX; ++cx)
            {
                /*
                futures.push_back(std::async(std::launch::async, [=, &config, &pixels, &model, &modelMutex]()
                {
                */
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;

                uint32_t startX = cx * config.chunkSize;
                uint32_t startZ = cz * config.chunkSize;

                uint32_t endX = std::min(startX + config.chunkSize, config.segmentsWidth);
                uint32_t endZ = std::min(startZ + config.chunkSize, config.segmentsDepth);

                uint32_t vCountX = (endX - startX) + 1;
                uint32_t vCountZ = (endZ - startZ) + 1;

                vertices.reserve(vCountX * vCountZ);

                for (uint32_t z = 0; z < vCountZ; ++z)
                {
                    for (uint32_t x = 0; x < vCountX; ++x)
                    {
                        uint32_t globalX = startX + x;
                        uint32_t globalZ = startZ + z;

                        Vertex v;

                        float u = static_cast<float>(globalX) / static_cast<float>(config.segmentsWidth);
                        float v_coord = static_cast<float>(globalZ) / static_cast<float>(config.segmentsDepth);

                        float hVal = 0.0f;
                        if (hasData)
                        {
                            hVal = GetHeightFromPixel(pixels, imgW, imgH, channels, u, v_coord);
                        }

                        v.position.x = (globalX * stepX) - halfW;
                        v.position.y = config.minHeight + (hVal * heightRange);
                        v.position.z = (globalZ * stepZ) - halfD;

                        v.texCoord = { u, 1.0f - v_coord };
                        v.tangent = { 1.0f, 0.0f, 0.0f, 1.0f };

                        float hL = hVal;
                        float hR = hVal;
                        float hD = hVal;
                        float hU = hVal;

                        // Left neighbor
                        if (globalX > 0)
                            hL = GetHeightFromPixel(
                                pixels, imgW, imgH, channels, u - (1.0f / config.segmentsWidth), v_coord);
                        // Right neighbor
                        if (globalX < config.segmentsWidth)
                            hR = GetHeightFromPixel(
                                pixels, imgW, imgH, channels, u + (1.0f / config.segmentsWidth), v_coord);
                        // Bottom neighbor
                        if (globalZ > 0)
                            hD = GetHeightFromPixel(
                                pixels, imgW, imgH, channels, u, v_coord - (1.0f / config.segmentsDepth));
                        // Top neighbor
                        if (globalZ < config.segmentsDepth)
                            hU = GetHeightFromPixel(
                                pixels, imgW, imgH, channels, u, v_coord + (1.0f / config.segmentsDepth));

                        hL = config.minHeight + hL * heightRange;
                        hR = config.minHeight + hR * heightRange;
                        hD = config.minHeight + hD * heightRange;
                        hU = config.minHeight + hU * heightRange;

                        float dX = hR - hL;
                        float dZ = hU - hD;

                        float distWidth = stepX * 2.0f;
                        float distDepth = stepZ * 2.0f;

                        Math::Vector3 n;
                        n.x = -dX / distWidth;
                        n.y = 1.0f;
                        n.z = -dZ / distDepth;
                        v.normal = Math::Normalize(n);

                        vertices.push_back(v);
                    }
                }

                for (uint32_t z = 0; z < vCountZ - 1; ++z)
                {
                    for (uint32_t x = 0; x < vCountX - 1; ++x)
                    {
                        uint32_t topLeft = z * vCountX + x;
                        uint32_t topRight = topLeft + 1;
                        uint32_t bottomLeft = (z + 1) * vCountX + x;
                        uint32_t bottomRight = bottomLeft + 1;

                        indices.push_back(topLeft);
                        indices.push_back(bottomLeft);
                        indices.push_back(topRight);

                        indices.push_back(topRight);
                        indices.push_back(bottomLeft);
                        indices.push_back(bottomRight);
                    }
                }

                //{
                //  std::lock_guard<std::mutex> lock(modelMutex);
                AddMeshToModel(model, vertices, indices);
                //}
                //}));
            }
        }

        // for (auto& f : futures)
        //     f.get();

        model->SetStatus(AssetStatus::Loaded);

        return model;
    }

    std::shared_ptr<Model> ModelFactory::CreateCube(const Component::MeshSourceCube& config)
    {
        if (config.bevelRadius > 0.001f)
        {
            return _CreateCubeWithBevel(config.size, config.bevelRadius);
        }

        Vector3 nbVertices = { std::max(1.0f, config.segments.x) + 1.0f,
                               std::max(1.0f, config.segments.y) + 1.0f,
                               std::max(1.0f, config.segments.z) + 1.0f };

        return _CreateCubeWithPrecision(config.size, nbVertices);
    }

    std::shared_ptr<Model> ModelFactory::_CreateCubeWithBevel(float size, float bevelSize)
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
        model->SetStatus(AssetStatus::Loaded);

        return model;
    }

    std::shared_ptr<Model> ModelFactory::_CreateCubeWithPrecision(float size, const Vector3& nbVertices)
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
        model->SetStatus(AssetStatus::Loaded);

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

        indices = { 0, 3, 2, 0, 2, 1 };

        AddMeshToModel(model, vertices, indices);
        model->SetStatus(AssetStatus::Loaded);

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
        model->SetStatus(AssetStatus::Loaded);

        return model;
    }

    std::shared_ptr<Model> ModelFactory::CreateCylinder(float bottomRadius,
                                                        float topRadius,
                                                        float height,
                                                        uint32_t sliceCount,
                                                        uint32_t stackCount)
    {
        auto model = std::make_shared<Model>();
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        float stackHeight = height / stackCount;
        float radiusStep = (topRadius - bottomRadius) / stackCount;
        uint32_t ringCount = sliceCount + 1;

        for (uint32_t i = 0; i <= stackCount; ++i)
        {
            float y = -0.5f * height + i * stackHeight;
            float r = bottomRadius + i * radiusStep;

            float dRadius = bottomRadius - topRadius;
            float dY = height;
            float slopeLen = std::sqrt(dRadius * dRadius + dY * dY);
            float ny = dRadius / slopeLen;
            float nz_factor = dY / slopeLen;

            float thetaStep = 2.0f * std::numbers::pi_v<float> / sliceCount;

            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;
                float c = std::cos(theta);
                float s = std::sin(theta);

                Vector3 pos = { r * c, y, r * s };

                Vector3 normal = { c * nz_factor, ny, s * nz_factor };
                Vector3 tangent = { -s, 0.0f, c };

                float u = (float)j / sliceCount;
                float v = 1.0f - (float)i / stackCount;

                vertices.push_back({ pos, normal, { u, v }, { tangent.x, tangent.y, tangent.z, 1.0f } });
            }
        }

        for (uint32_t i = 0; i < stackCount; ++i)
        {
            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                uint32_t base = i * ringCount + j;

                indices.push_back(base);
                indices.push_back(base + ringCount);
                indices.push_back(base + ringCount + 1);

                indices.push_back(base);
                indices.push_back(base + ringCount + 1);
                indices.push_back(base + 1);
            }
        }

        // Top
        if (topRadius > 0.0f)
        {
            uint32_t centerIndex = (uint32_t)vertices.size();
            uint32_t ringStartIndex = centerIndex + 1;

            vertices.push_back({ { 0, 0.5f * height, 0 }, { 0, 1, 0 }, { 0.5f, 0.5f }, { 1, 0, 0, 1 } });

            float thetaStep = 2.0f * std::numbers::pi_v<float> / sliceCount;
            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;
                float x = topRadius * std::cos(theta);
                float z = topRadius * std::sin(theta);

                float u = x / height + 0.5f;
                float v = z / height + 0.5f;

                vertices.push_back({ { x, 0.5f * height, z }, { 0, 1, 0 }, { u, v }, { 1, 0, 0, 1 } });
            }

            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                indices.push_back(centerIndex);
                indices.push_back(ringStartIndex + j + 1);
                indices.push_back(ringStartIndex + j);
            }
        }

        // Bottom
        if (bottomRadius > 0.0f)
        {
            uint32_t centerIndex = (uint32_t)vertices.size();
            uint32_t ringStartIndex = centerIndex + 1;

            vertices.push_back({ { 0, -0.5f * height, 0 }, { 0, -1, 0 }, { 0.5f, 0.5f }, { 1, 0, 0, 1 } });

            float thetaStep = 2.0f * std::numbers::pi_v<float> / sliceCount;
            for (uint32_t j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;
                float x = bottomRadius * std::cos(theta);
                float z = bottomRadius * std::sin(theta);

                float u = x / height + 0.5f;
                float v = z / height + 0.5f;

                vertices.push_back({ { x, -0.5f * height, z }, { 0, -1, 0 }, { u, v }, { 1, 0, 0, 1 } });
            }

            for (uint32_t j = 0; j < sliceCount; ++j)
            {
                indices.push_back(centerIndex);
                indices.push_back(ringStartIndex + j);
                indices.push_back(ringStartIndex + j + 1);
            }
        }

        AddMeshToModel(model, vertices, indices);
        model->SetStatus(AssetStatus::Loaded);

        return model;
    }
} // namespace Frost