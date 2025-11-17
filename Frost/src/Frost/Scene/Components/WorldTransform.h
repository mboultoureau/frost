#pragma once

#include "Frost/Scene/Components/Transform.h"

namespace Frost::Component
{
	struct WorldTransform : public Transform
	{
		WorldTransform() noexcept
			: Transform()
		{
		}

		WorldTransform(const Math::Vector3& pos, const Math::Vector4& rot, const Math::Vector3& scl) noexcept
			: Transform(pos, rot, scl)
		{
		}

		WorldTransform(const Math::Vector3& pos) noexcept
			: Transform(pos)
		{
		}
	};
}