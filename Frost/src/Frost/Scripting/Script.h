#pragma once

#include "Frost/Physics/PhysicListener.h"
#include "Frost/Scene/ECS/GameObject.h"

namespace Frost::Scripting
{
    class ECS;

    class Script
    {
    public:
        // Lifecycle
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnPreFixedUpdate(float fixedDeltaTime) {}
        virtual void OnFixedUpdate(float fixedDeltaTime) {}
        virtual void OnLateUpdate(float deltaTime) {}

        // Physics
        virtual void OnAwake(float deltaTime) {}
        virtual void OnSleep(float deltaTime) {}
        virtual void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) {}
        virtual void OnCollisionStay(BodyOnContactParameters params, float deltaTime) {}
        virtual void OnCollisionExit(std::pair<entt::entity, entt::entity> params, float deltaTime) {}

        // Accessors
        GameObject GetGameObject() { return _gameObject; }
        void SetGameObject(GameObject gameObject) { _gameObject = gameObject; }
        Scene* GetScene() { return _gameObject.GetScene(); }

    protected:
        GameObject _gameObject;
    };
} // namespace Frost::Scripting