#pragma once

#include "Frost/Scene/ECS/System.h"

namespace Frost
{
	class UISystem : public System
	{
	public:
		UISystem();
		void Update(Frost::ECS& ecs, float deltaTime) override;

	private:

	};
}