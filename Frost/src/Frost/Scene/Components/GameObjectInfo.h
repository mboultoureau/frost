#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Scene/ECS/GameObject.h"

#include <string>

namespace Frost
{
	struct GameObjectInfo : public Component
	{
		GameObject::Id id = 0;
		GameObject::Id parentId = GameObject::InvalidId;
		std::string name;

		GameObjectInfo(GameObject::Id id, const std::string& name)
			: id(id), name(name)
		{
		}
	};
}
