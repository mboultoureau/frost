#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Scene/ECS/GameObject.h"

#include <string>

namespace Frost::Component
{
	struct Meta : public Component
	{
		std::string name;

		Meta(std::string name = "GameObject")
			: name(std::move(name))
		{
		}
	};
}
