#pragma once

#include "Frost/Core/Timer.h"
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
		Scene(std::string&& name);

		GameObject::Id CreateGameObject();
		GameObject::Id CreateGameObject(std::string name);
		void DestroyGameObject(GameObject::Id id);

		void Update(float deltaTime);
		void FixedUpdate(float deltaTime);
		void LateUpdate(float deltaTime);

		template <typename T, typename... Args>
		void AddScript(GameObject::Id id, Args&&... args);

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

		const std::string& GetName() const { return _name; }
		ECS& GetECS() { return _ecs; }

	private:
		ECS _ecs;
		std::string _name;
		std::vector<std::unique_ptr<System>> _systems;
		void _InitializeSystems();
	};

	template <typename T, typename... Args>
	void Scene::AddScript(GameObject::Id id, Args&&... args)
	{
		static_assert(std::is_base_of<Script, T>::value, "Type T must inherit from Frost::Script.");
		std::unique_ptr<Script> script = std::make_unique<T>(std::forward<Args>(args)...);
		_ecs.AddScript(id, std::move(script));
	}
}