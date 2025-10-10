#pragma once

#include "Scene/ECS/ECS.h"

namespace Frost
{
	enum class SystemType
	{
		Update,
		FixedUpdate,
		LateUpdate
	};

	class ISystem
	{
	public:
		virtual ~ISystem() = default;
		virtual void Update(ECS& ecs, float deltaTime) = 0;
		virtual SystemType GetSystemType() const = 0;
	};
}
