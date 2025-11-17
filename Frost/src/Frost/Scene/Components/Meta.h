#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Scene/ECS/GameObject.h"

#include <string>

namespace Frost::Component
{
	struct Meta : public Component
	{
		GameObject::Id id = 0;
		GameObject::Id parentId = GameObject::InvalidId;
		std::string name;

		Meta(GameObject::Id id, const std::string& name)
			: id(id), name(name)
		{
		}

		Meta(GameObject::Id id, GameObject::Id parentId, const std::string& name)
			: id(id), name(name), parentId(parentId)
		{
		}
	};
}
