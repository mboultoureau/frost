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
			OnAwake();
		}
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}

		virtual void OnAwake() {}
		virtual void OnSleep() {}

		virtual void OnCollisionEnter(GameObject::Id otherObject) {}
		virtual void OnCollisionStay(GameObject::Id otherObject) {}
		virtual void OnCollisionExit(GameObject::Id otherObject) {}

		virtual ECS* GetECS() const { return _ecs; }
		virtual GameObject::Id GetGameObject() const { return _gameObject; }
	private:
		GameObject::Id _gameObject = GameObject::InvalidId;
		ECS* _ecs = nullptr;
	};
}