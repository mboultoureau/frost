#include "Frost/Asset/ModelFactory.h"
#include "Frost/Asset/AssetManager.h"
#include "Frost/Utils/Math/Vector.h"

using namespace Frost::Math;

namespace Frost
{
    // Helper function to add a mesh
    static void AddMeshToModel(std::shared_ptr<Model>& model, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
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
				vertices.push_back({ c0, normal, {0.0f, 1.0f}, {tangent.x, tangent.y, tangent.z, 1.0f} });
				vertices.push_back({ c1, normal, {1.0f, 1.0f}, {tangent.x, tangent.y, tangent.z, 1.0f} });
				vertices.push_back({ c2, normal, {1.0f, 0.0f}, {tangent.x, tangent.y, tangent.z, 1.0f} });
				vertices.push_back({ c3, normal, {0.0f, 0.0f}, {tangent.x, tangent.y, tangent.z, 1.0f} });

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

	std::shared_ptr<Model> ModelFactory::CreatePlane(float width, float depth)
	{
		auto model = std::make_shared<Model>();
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		float hw = width * 0.5f;
		float hd = depth * 0.5f;

		Vector3 n = { 0.0f, 1.0f, 0.0f };
		Vector3 t = { 1.0f, 0.0f, 0.0f };

		vertices.push_back({ {-hw, 0.0f, -hd}, n, {0.0f, 1.0f}, {t.x, t.y, t.z, 1.0f} }); // BL
		vertices.push_back({ { hw, 0.0f, -hd}, n, {1.0f, 1.0f}, {t.x, t.y, t.z, 1.0f} }); // BR
		vertices.push_back({ { hw, 0.0f,  hd}, n, {1.0f, 0.0f}, {t.x, t.y, t.z, 1.0f} }); // TR
		vertices.push_back({ {-hw, 0.0f,  hd}, n, {0.0f, 0.0f}, {t.x, t.y, t.z, 1.0f} }); // TL

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

				vertices.push_back({ pos, normal, {u, v}, {tangent.x, tangent.y, tangent.z, 1.0f} });
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
}