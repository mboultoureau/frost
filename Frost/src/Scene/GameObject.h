#pragma once

#include "Transform.h"
#include "Component.h"

namespace Frost
{
	class GameObject
	{
		using Id = size_t;

	public:
		GameObject();
		GameObject(GameObject& parent);
		void SetParent(GameObject& newParent);
		void AddComponent(Component::Id newComponent) { _components.push_back(newComponent); };
		void RemoveComponent();


	private:
		size_t _id;
		Transform _transform;
		std::vector<Component::Id> _components;
		GameObject &parent;
		
	};
}
