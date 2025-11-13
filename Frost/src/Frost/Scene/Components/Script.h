#pragma once

#include "Frost/Scene/ECS/GameObject.h"
#include "Frost/Physics/PhysicListener.h"

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
			OnInitialize();
		}
		// Called after constructor, so you have gameobject and ecs that are accessible
		virtual void OnInitialize() {}

		virtual void OnUpdate(float deltaTime) {}
		virtual void OnFixedUpdate(float fixedDeltaTime) {}
		virtual void OnLateUpdate(float deltaTime) {}

		virtual void OnAwake(float deltaTime) {}
		virtual void OnSleep(float deltaTime) {}

		virtual void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) {}
		virtual void OnCollisionStay(BodyOnContactParameters params, float deltaTime) {}

		// Warning : params may contains bodies that are not valid at the moment
		virtual void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) {}

		virtual ECS* GetECS() const { return _ecs; }
		virtual GameObject::Id GetGameObject() const { return _gameObject; }
	private:
		GameObject::Id _gameObject = GameObject::InvalidId;
		ECS* _ecs = nullptr;
	};
}