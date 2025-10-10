#pragma once

#include <DirectXMath.h>

namespace Frost
{
	struct Transform
	{
		using Vector3 = DirectX::XMFLOAT3;

		Vector3 position;
		Vector3 rotation;
		Vector3 scale;
	};
}
