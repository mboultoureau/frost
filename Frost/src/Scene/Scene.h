#pragma once

#include "Scene/ECS/ECS.h"
#include "Scene/ECS/GameObject.h"
#include "Scene/ECS/System.h"

#include <vector>
#include <memory>

namespace Frost
{
	class Scene
	{
	public:
		Scene();

		GameObject::Id CreateGameObject();
		void DestroyGameObject(GameObject::Id id);

		void Update(float deltaTime);
		void FixedUpdate(float deltaTime);
		void LateUpdate(float deltaTime);

	private:
		ECS _ecs;
		std::vector<std::unique_ptr<ISystem>> _systems;
		void _InitializeSystems();
	};
}