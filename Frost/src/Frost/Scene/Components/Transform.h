#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <DirectXMath.h>

namespace Frost
{
	struct Transform : public Component
	{
		using Vector3 = DirectX::XMFLOAT3;

		Vector3 position;
		Vector3 rotation;
		Vector3 scale;

		Transform() noexcept
			: position(0.0f, 0.0f, 0.0f), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f)
		{
		}

		Transform(const Vector3& pos, const Vector3& rot, const Vector3& scl) noexcept
			: position(pos), rotation(rot), scale(scl)
		{
		}

		Transform(const Vector3& pos) noexcept
			: position(pos), rotation(0.0f, 0.0f, 0.0f), scale(1.0f, 1.0f, 1.0f)
		{
		}
	};
}
