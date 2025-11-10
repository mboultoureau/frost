#include "HeightMapModel.h"
#include "Frost/Renderer/Vertex.h"
#include "Frost/Renderer/TextureLibrary.h"
#include "Frost/Renderer/Mesh.h"

#include <vector>
#include <Frost.h>

namespace Frost
{
    HeightMapModel::HeightMapModel(
        const std::string& heightmapPath,
        Material material,
        TextureChannel channel,
        int chunkSize,
        float heightScale
    )
    {
        _filepath = heightmapPath;
        auto abstractTexture = TextureLibrary::Get().GetTexture(heightmapPath, TextureType::UNKNOWN);
        TextureDX11* texture = static_cast<TextureDX11*>(abstractTexture.get());
        if (!texture)
        {
            FT_ENGINE_ERROR("Heightmap load failed: {}", heightmapPath);
            throw std::runtime_error("Heightmap loading failed");
        }

        GenerateFromHeightmap(texture, material, channel, chunkSize, heightScale);
    }

    void HeightMapModel::GenerateFromHeightmap(
        TextureDX11* heightTexture,
        const Material& material,
        TextureChannel channel,
        int chunkSize,
        float heightScale
    )
    {
        width = heightTexture->GetWidth();
        height = heightTexture->GetHeight();

        /*GenerateMeshFromHeightmap(
            chunkSize, 2*chunkSize,
            chunkSize, 2*chunkSize,
            heightTexture,
            material,
            channel,
            heightScale
        );*/

        
        for (int z = 0; z < height; z += chunkSize)
        {
            for (int x = 0; x < width; x += chunkSize)
            {
                GenerateMeshFromHeightmap(
                    x, x + chunkSize,
                    z, z + chunkSize,
                    heightTexture,
                    material,
                    channel,
                    heightScale
                );
            }
        }
    }
    
    void HeightMapModel::GenerateMeshFromHeightmap(
        int xMin,
        int xMax,
        int zMin,
        int zMax,
        TextureDX11* heightTexture,
        const Material& material,
        TextureChannel channel,
        float heightScale
    )
    {
        using namespace DirectX;
        FT_ENGINE_ASSERT(heightTexture && heightTexture->GetHeight() > 0 && heightTexture->GetWidth() > 0, "Failed to load height map texture {}", _filepath);

        _materials.clear();
        _materials.push_back(material);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        xMin = xMin - 1 < 0 ? 0 : xMin - 1;
        xMax = xMax + 1 > width ? width : xMax + 1;

        zMin = zMin - 1 < 0 ? 0 : zMin - 1;
        zMax = zMax + 1 > height ? height : zMax + 1;

        int localWidth = (xMax - xMin);
        int localHeight = (zMax - zMin);

        vertices.reserve(localWidth * localHeight);

        // --- Lire la hauteur des pixels ---
        auto data = heightTexture->GetTextureRawData(); // doit exposer l'accès brut

        auto getPos = [&](int px, int pz)
            {
                // Clamp aux bords
          /*      px = std::clamp(px, xMin, xMax);
                px = std::clamp(px, 0, width - 1);

                pz = std::clamp(pz, zMin, zMax);
                pz = std::clamp(pz, 0, height - 1);*/

                int pidx = (pz * width + px) * 4;
                float ph = 0.0f;
                switch (channel)
                {
                case TextureChannel::R: ph = data[pidx + 0] / 255.0f; break;
                case TextureChannel::G: ph = data[pidx + 1] / 255.0f; break;
                case TextureChannel::B: ph = data[pidx + 2] / 255.0f; break;
                case TextureChannel::A: ph = data[pidx + 3] / 255.0f; break;
                }

                float worldX = (float)px / width;
                float worldZ = (float)pz / height;
                float worldY = ph * heightScale;

                return XMVECTOR{ worldX, worldY, worldZ, 0 };
            };

        for (int z = zMin; z < zMax; z++)
        {
            for (int x = xMin ; x < xMax; x++)
            {
                int idx = (z * width + x) * 4;
                float h = 0.0f;

                switch (channel)
                {
                case TextureChannel::R: h = data[idx + 0] / 255.0f; break;
                case TextureChannel::G: h = data[idx + 1] / 255.0f; break;
                case TextureChannel::B: h = data[idx + 2] / 255.0f; break;
                case TextureChannel::A: h = data[idx + 3] / 255.0f; break;
                }

                float worldX = (float)x / width - 0.5f;
                float worldZ = (float)z / height - 0.5f;
                float worldY = h * heightScale;

                XMVECTOR p0 = { worldX, worldY, worldZ, 0 };

                // Vecteurs voisins
                XMVECTOR v1 = { 0,0,0,0 };
                XMVECTOR v2 = { 0,0,0,0 };
                XMVECTOR v3 = { 0,0,0,0 };
                XMVECTOR v4 = { 0,0,0,0 };

                bool hasUp = (z < height - 1);
                bool hasRight = (x < width - 1);
                bool hasDown = (z > 0);
                bool hasLeft = (x > 0);

                if (hasUp)    v1 = getPos(x, z + 1) - p0;
                if (hasRight) v2 = getPos(x + 1, z) - p0;
                if (hasDown)  v3 = getPos(x, z - 1) - p0;
                if (hasLeft)  v4 = getPos(x - 1, z) - p0;

                XMVECTOR n1 = XMVectorZero();
                XMVECTOR n2 = XMVectorZero();
                XMVECTOR n3 = XMVectorZero();
                XMVECTOR n4 = XMVectorZero();

                // Produits vectoriels
                if (hasUp && hasRight) n1 = XMVector3Cross(v2, v1);
                if (hasDown && hasRight) n2 = XMVector3Cross(v3, v2);
                if (hasDown && hasLeft)  n3 = XMVector3Cross(v4, v3);
                if (hasUp && hasLeft)  n4 = XMVector3Cross(v1, v4);

                // Somme -> normalisation
                XMVECTOR normalVec = n1 + n2 + n3 + n4;
                normalVec = XMVector3Normalize(normalVec);

                XMFLOAT3 normal;
                XMStoreFloat3(&normal, -1*normalVec);

                Vertex v{};
                v.position = { worldX, worldY, worldZ };
                v.normal = normal;
                v.texCoord = { (float)x / width, (float)z / height };

                vertices.push_back(v);
            }
        }
        // --- Indices (grid triangulé) ---

        auto a = vertices.size();
        auto b = localHeight * localWidth;

        for (int z = 0; z < localHeight -1  ; z++)
        {
            for (int x = 0; x < localWidth-1 ; x++)
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

        Mesh mesh(vertices, indices);
        mesh.SetMaterialIndex(0); // le seul material

        _meshes.push_back(mesh);
    }
}
