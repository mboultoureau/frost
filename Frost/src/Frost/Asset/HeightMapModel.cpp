#include "Frost/Asset/HeightMapModel.h"
#include "Frost/Debugging/Assert.h"

namespace Frost
{
    HeightMapModel::HeightMapModel(const HeightMapConfig& config) : _config{ config }
    {
        FT_ENGINE_ASSERT(config.texture, "HeightMapModel: Heightmap texture is null.");
        FT_ENGINE_ASSERT(config.chunkSize > 0, "HeightMapModel: Chunk size must be greater than zero.");
        FT_ENGINE_ASSERT(config.texture->GetWidth() > 0 && config.texture->GetHeight() > 0,
                         "HeightMapModel: Heightmap texture has invalid dimensions.");

        uint32_t imageWidth = config.texture->GetWidth();
        uint32_t imageHeight = config.texture->GetHeight();

        _materials.push_back(config.material);

        for (int z = 0; z < imageHeight; z += config.chunkSize)
        {
            for (int x = 0; x < imageWidth; x += config.chunkSize)
            {
                GenerateHeightMapMesh(x, x + config.chunkSize, z, z + config.chunkSize, config.scale.height);
            }
        }
    }

    void HeightMapModel::GenerateHeightMapMesh(uint32_t xMin,
                                               uint32_t xMax,
                                               uint32_t zMin,
                                               uint32_t zMax,
                                               float heightScale)
    {
        using namespace DirectX;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        xMin = xMin == 0 ? 0 : xMin - 1;
        xMax = std::min(xMax + 1, _config.texture->GetWidth());

        zMin = zMin == 0 ? 0 : zMin - 1;
        zMax = std::min(zMax + 1, _config.texture->GetHeight());

        int localWidth = (xMax - xMin);
        int localHeight = (zMax - zMin);

        vertices.reserve(localWidth * localHeight);
        auto data = _config.texture->GetData();

        auto getPos = [&](int px, int pz)
        {
            int pidx = (pz * _config.texture->GetWidth() + px) * _config.texture->GetChannels();
            float ph = data[pidx] / 255.0f;

            float worldX = (float)px / _config.texture->GetWidth();
            float worldZ = (float)pz / _config.texture->GetHeight();
            float worldY = ph * heightScale;

            return XMVECTOR{ worldX, worldY, worldZ, 0 };
        };

        for (int z = zMin; z < zMax; z++)
        {
            for (int x = xMin; x < xMax; x++)
            {
                int idx = (z * _config.texture->GetWidth() + x) * _config.texture->GetChannels();
                float h = data[idx] / 255.0f;

                float worldX = (float)x / _config.texture->GetWidth() - 0.5f;
                float worldZ = (float)z / _config.texture->GetHeight() - 0.5f;
                float worldY = h * _config.scale.height;

                XMVECTOR p0 = { worldX, worldY, worldZ, 0 };

                // Vecteurs voisins
                XMVECTOR v1 = { 0, 0, 0, 0 };
                XMVECTOR v2 = { 0, 0, 0, 0 };
                XMVECTOR v3 = { 0, 0, 0, 0 };
                XMVECTOR v4 = { 0, 0, 0, 0 };

                bool hasUp = (z < _config.texture->GetHeight() - 1);
                bool hasRight = (x < _config.texture->GetWidth() - 1);
                bool hasDown = (z > 0);
                bool hasLeft = (x > 0);

                if (hasUp)
                    v1 = XMVectorSubtract(getPos(x, z + 1), p0);
                if (hasRight)
                    v2 = XMVectorSubtract(getPos(x + 1, z), p0);
                if (hasDown)
                    v3 = XMVectorSubtract(getPos(x, z - 1), p0);
                if (hasLeft)
                    v4 = XMVectorSubtract(getPos(x - 1, z), p0);

                XMVECTOR n1 = XMVectorZero();
                XMVECTOR n2 = XMVectorZero();
                XMVECTOR n3 = XMVectorZero();
                XMVECTOR n4 = XMVectorZero();

                // Produits vectoriels
                if (hasUp && hasRight)
                    n1 = XMVector3Cross(v2, v1);
                if (hasDown && hasRight)
                    n2 = XMVector3Cross(v3, v2);
                if (hasDown && hasLeft)
                    n3 = XMVector3Cross(v4, v3);
                if (hasUp && hasLeft)
                    n4 = XMVector3Cross(v1, v4);

                // Somme -> normalisation
                XMVECTOR normalVec = XMVectorAdd(XMVectorAdd(XMVectorAdd(n1, n2), n3), n4);
                normalVec = XMVector3Normalize(-1 * normalVec);

                XMFLOAT3 normal;
                XMStoreFloat3(&normal, normalVec);

                Vertex v{};
                v.position = { worldX, worldY, worldZ };
                v.normal = Math::vector_cast<Math::Vector3>(normal);
                v.texCoord = { (float)x / _config.texture->GetWidth(), (float)z / _config.texture->GetHeight() };

                vertices.push_back(v);
            }
        }

        auto a = vertices.size();
        auto b = localHeight * localWidth;

        for (int z = 0; z < localHeight - 1; z++)
        {
            for (int x = 0; x < localWidth - 1; x++)
            {
                int topLeft = z * localWidth + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * localWidth + x;
                int bottomRight = bottomLeft + 1;

                auto vtopL = vertices[topLeft].position;
                auto vtopR = vertices[topRight].position;
                auto vBL = vertices[bottomLeft].position;
                auto vBR = vertices[bottomRight].position;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        // Tangents
        std::vector<XMVECTOR> tangents(vertices.size(), XMVectorZero());
        std::vector<XMVECTOR> bitangents(vertices.size(), XMVectorZero());

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            XMVECTOR v0 = XMVectorSet(vertices[i0].position.x, vertices[i0].position.y, vertices[i0].position.z, 0.0f);
            XMVECTOR v1 = XMVectorSet(vertices[i1].position.x, vertices[i1].position.y, vertices[i1].position.z, 0.0f);
            XMVECTOR v2 = XMVectorSet(vertices[i2].position.x, vertices[i2].position.y, vertices[i2].position.z, 0.0f);

            XMVECTOR uv0 = XMVectorSet(vertices[i0].texCoord.x, vertices[i0].texCoord.y, 0.0f, 0.0f);
            XMVECTOR uv1 = XMVectorSet(vertices[i1].texCoord.x, vertices[i1].texCoord.y, 0.0f, 0.0f);
            XMVECTOR uv2 = XMVectorSet(vertices[i2].texCoord.x, vertices[i2].texCoord.y, 0.0f, 0.0f);

            XMVECTOR edge1 = XMVectorSubtract(v1, v0);
            XMVECTOR edge2 = XMVectorSubtract(v2, v0);

            XMVECTOR deltaUV1 = XMVectorSubtract(uv1, uv0);
            XMVECTOR deltaUV2 = XMVectorSubtract(uv2, uv0);

            float denominator =
                XMVectorGetX(deltaUV1) * XMVectorGetY(deltaUV2) - XMVectorGetY(deltaUV1) * XMVectorGetX(deltaUV2);
            float f = (fabs(denominator) < 1e-6f) ? 1.0f : (1.0f / denominator);

            float deltaUV2Y = XMVectorGetY(deltaUV2);
            float deltaUV1Y = XMVectorGetY(deltaUV1);
            float deltaUV1X = XMVectorGetX(deltaUV1);
            float deltaUV2X = XMVectorGetX(deltaUV2);

            XMVECTOR tangent =
                XMVectorScale(XMVectorSubtract(XMVectorScale(edge1, deltaUV2Y), XMVectorScale(edge2, deltaUV1Y)), f);
            XMVECTOR bitangent =
                XMVectorScale(XMVectorSubtract(XMVectorScale(edge2, deltaUV1X), XMVectorScale(edge1, deltaUV2X)), f);

            tangents[i0] = XMVectorAdd(tangents[i0], tangent);
            tangents[i1] = XMVectorAdd(tangents[i1], tangent);
            tangents[i2] = XMVectorAdd(tangents[i2], tangent);

            bitangents[i0] = XMVectorAdd(bitangents[i0], bitangent);
            bitangents[i1] = XMVectorAdd(bitangents[i1], bitangent);
            bitangents[i2] = XMVectorAdd(bitangents[i2], bitangent);
        }

        for (size_t i = 0; i < vertices.size(); i++)
        {
            XMVECTOR normal = XMVectorSet(vertices[i].normal.x, vertices[i].normal.y, vertices[i].normal.z, 0.0f);
            XMVECTOR tangent = tangents[i];

            // Check if tangent is zero vector
            float tangentLength = XMVectorGetX(XMVector3Length(tangent));
            if (tangentLength < 1e-6f)
            {
                XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
                tangent = XMVector3Cross(normal, up);
                tangentLength = XMVectorGetX(XMVector3Length(tangent));
                if (tangentLength < 1e-6f)
                {
                    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
                    tangent = XMVector3Cross(normal, right);
                }
            }

            tangent = XMVector3Normalize(tangent);

            float dotProduct = XMVectorGetX(XMVector3Dot(normal, tangent));
            tangent = XMVector3Normalize(XMVectorSubtract(tangent, XMVectorScale(normal, dotProduct)));

            XMVECTOR bitangent = bitangents[i];
            float bitangentLength = XMVectorGetX(XMVector3Length(bitangent));
            if (bitangentLength < 1e-6f)
            {
                bitangent = XMVector3Cross(normal, tangent);
            }
            else
            {
                bitangent = XMVector3Normalize(bitangent);
            }

            XMVECTOR crossResult = XMVector3Cross(normal, tangent);
            float dotResult = XMVectorGetX(XMVector3Dot(crossResult, bitangent));
            float handedness = (dotResult < 0.0f) ? -1.0f : 1.0f;

            XMFLOAT4 tangentResult;
            XMStoreFloat4(&tangentResult, XMVectorSetW(tangent, handedness));

            vertices[i].tangent = Math::vector_cast<Math::Vector4>(tangentResult);
        }

        auto vertexData = std::as_bytes(std::span(vertices));
        _meshes.emplace_back(vertexData, static_cast<uint32_t>(sizeof(Vertex)), indices);
        _meshes.back().SetMaterialIndex(0);
    }
} // namespace Frost