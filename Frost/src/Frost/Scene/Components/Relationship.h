#pragma once

#include "Frost/Scene/ECS/Component.h"

#include <entt/entt.hpp>

/**
* @see https://skypjack.github.io/2019-06-25-ecs-baf-part-4/
*/
namespace Frost::Component
{
	struct Relationship : public Component
	{
		std::size_t childrenCount{};
		entt::entity firstChild{ entt::null };
		entt::entity prevSibling{ entt::null };
		entt::entity nextSibling{ entt::null };
		entt::entity parent{ entt::null };
	};
}