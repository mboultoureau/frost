#include "Mesh.h"

#include <DirectXMath.h>

namespace Frost
{
	Mesh::Mesh(const std::string& filepath) : _filepath{ filepath }
	{
		const uint16_t index_bloc[36] = {
			0,1,2,
			0,2,3,
			5,6,7,
			5,7,4,
			8,9,10,
			8,10,11, 
			13,14,15,
			13,15,12,
			19,16,17,
			19,17,18,
			20,21,22,
			20,22,23 
		};

		float dx = 1.0f;
		float dy = 1.0f;
		float dz = 1.0f;

		DirectX::XMFLOAT3 point[8] =
		{
			DirectX::XMFLOAT3(-dx / 2, dy / 2, -dz / 2),
			DirectX::XMFLOAT3(dx / 2, dy / 2, -dz / 2),
			DirectX::XMFLOAT3(dx / 2, -dy / 2, -dz / 2),
			DirectX::XMFLOAT3(-dx / 2, -dy / 2, -dz / 2),
			DirectX::XMFLOAT3(-dx / 2, dy / 2, dz / 2),
			DirectX::XMFLOAT3(-dx / 2, -dy / 2, dz / 2),
			DirectX::XMFLOAT3(dx / 2, -dy / 2, dz / 2),
			DirectX::XMFLOAT3(dx / 2, dy / 2, dz / 2)
		};


		// Calculer les normales
		DirectX::XMFLOAT3 n0(0.0f, 0.0f, -1.0f); // devant
		DirectX::XMFLOAT3 n1(0.0f, 0.0f, 1.0f); // arrière
		DirectX::XMFLOAT3 n2(0.0f, -1.0f, 0.0f); // dessous
		DirectX::XMFLOAT3 n3(0.0f, 1.0f, 0.0f); // dessus
		DirectX::XMFLOAT3 n4(-1.0f, 0.0f, 0.0f); // face gauche
		DirectX::XMFLOAT3 n5(1.0f, 0.0f, 0.0f); // face droite 

		struct CSommetBloc
		{
			const DirectX::XMFLOAT3& position;
			const DirectX::XMFLOAT3& normal;
			const DirectX::XMFLOAT2& coordTex;
		};

		CSommetBloc sommets[24] =
		{
			// Le devant du bloc
			CSommetBloc(point[0], n0, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[1], n0, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[2], n0, DirectX::XMFLOAT2(1.0f, 1.0f)),
			CSommetBloc(point[3], n0, DirectX::XMFLOAT2(0.0f, 1.0f)),
			// L’arrière du bloc
			CSommetBloc(point[4], n1, DirectX::XMFLOAT2(0.0f, 1.0f)),
			CSommetBloc(point[5], n1, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[6], n1, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[7], n1, DirectX::XMFLOAT2(1.0f, 1.0f)),
			// Le dessous du bloc
			CSommetBloc(point[3], n2, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[2], n2, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[6], n2, DirectX::XMFLOAT2(1.0f, 1.0f)),
			CSommetBloc(point[5], n2, DirectX::XMFLOAT2(0.0f, 1.0f)),
			// Le dessus du bloc
			CSommetBloc(point[0], n3, DirectX::XMFLOAT2(0.0f, 1.0f)),
			CSommetBloc(point[4], n3, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[7], n3, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[1], n3, DirectX::XMFLOAT2(1.0f, 1.0f)),
			// La face gauche
			CSommetBloc(point[0], n4, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[3], n4, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[5], n4, DirectX::XMFLOAT2(1.0f, 1.0f)),
			CSommetBloc(point[4], n4, DirectX::XMFLOAT2(0.0f, 1.0f)),
			// La face droite
			CSommetBloc(point[1], n5, DirectX::XMFLOAT2(0.0f, 0.0f)),
			CSommetBloc(point[7], n5, DirectX::XMFLOAT2(1.0f, 0.0f)),
			CSommetBloc(point[6], n5, DirectX::XMFLOAT2(1.0f, 1.0f)),
			CSommetBloc(point[2], n5, DirectX::XMFLOAT2(0.0f, 1.0f))
		};

		_vertexBuffer.Create(sommets, sizeof(sommets));
		_indexBuffer.Create(index_bloc, sizeof(index_bloc));
	}

	Mesh* MeshLibrary::Get(const std::string& filepath)
	{
		if (!Exists(filepath))
		{
			_meshes.emplace(filepath, Mesh(filepath));
		}
		return &_meshes.at(filepath);
	}

	bool MeshLibrary::Exists(const std::string& filepath) const
	{
		return _meshes.contains(filepath);
	}
}