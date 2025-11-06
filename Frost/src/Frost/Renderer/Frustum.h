#pragma once
#include <DirectXMath.h>
#include "Frost/Renderer/BoundingBox.h"

namespace Frost
{
	struct Plane {
		DirectX::XMFLOAT4 equation; // a, b, c, d
	};

	class Frustum {
	public:
		Plane planes[6];

		void Extract(const DirectX::XMMATRIX& viewProj, float margin);
		bool IsInside(const BoundingBox& box);
	};
}