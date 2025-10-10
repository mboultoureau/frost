#pragma once

#include "Frost/Scene/ECS/ECS.h"
#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Scene/ECS/System.h"

#include <vector>
#include <memory>
#include <string>

namespace Frost
{
	class Scene
	{
	public:
		Scene();

		GameObject::Id CreateGameObject();
		GameObject::Id CreateGameObject(std::string name);
		void DestroyGameObject(GameObject::Id id);

		void Update(float deltaTime);
		void FixedUpdate(float deltaTime);
		void LateUpdate(float deltaTime);

		template <typename T, typename... Args>
		void AddComponent(GameObject::Id id, Args&&... args)
		{
			_ecs.AddComponent<T>(id, std::forward<Args>(args)...);
		}

		template <typename T>
		void RemoveComponent(GameObject::Id id)
		{
			_ecs.RemoveComponent<T>(id);
		}

		template <typename T>
		T* GetComponent(GameObject::Id id)
		{
			return _ecs.GetComponent<T>(id);
		}

	private:
		ECS _ecs;
		std::vector<std::unique_ptr<System>> _systems;
		void _InitializeSystems();
	};
}