#pragma once

#include "Frost/Scene/ECS/GameObject.h"

namespace Frost
{
	class ECS;

	class Script
	{
	public:
		virtual void Initialize(GameObject::Id gameObject, ECS* ecs)
		{
			_gameObject = gameObject;
			_ecs = ecs;
		}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}

		virtual ECS* GetECS() const { return _ecs; }
		virtual GameObject::Id GetGameObject() const { return _gameObject; }

	private:
		GameObject::Id _gameObject = GameObject::InvalidId;
		ECS* _ecs = nullptr;
	};
}