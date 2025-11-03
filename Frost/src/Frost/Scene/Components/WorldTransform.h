#pragma once

#include "Frost/Scene/Components/Transform.h"

namespace Frost
{
	struct WorldTransform : public Transform
	{
		WorldTransform() noexcept
			: Transform()
		{
		}

		WorldTransform(const Vector3& pos, const Vector4& rot, const Vector3& scl) noexcept
			: Transform(pos, rot, scl)
		{
		}

		WorldTransform(const Vector3& pos) noexcept
			: Transform(pos)
		{
		}
	};
}