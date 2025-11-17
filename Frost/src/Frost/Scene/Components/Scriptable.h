#pragma once

#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Scene/Components/Script.h"

#include <vector>
#include <memory>

namespace Frost::Component
{
	struct Scriptable : public Component
	{
		std::vector<std::unique_ptr<Script>> _scripts;
	};
}