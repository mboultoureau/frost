#pragma once

#include <DirectXMath.h>

namespace Frost
{
	struct Vertex
	{
		using Vector3 = DirectX::XMFLOAT3;
		using Vector2 = DirectX::XMFLOAT2;

		Vector3 position;
		Vector3 normal;
		Vector2 texCoord;
		Vector3 tangent;
		Vector3 bitangent;
	};
}