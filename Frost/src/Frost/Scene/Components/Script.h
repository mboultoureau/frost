#pragma once

#include "Frost/Physics/PhysicListener.h"
#include "Frost/Scene/ECS/GameObject.h"

namespace Frost
{
    class ECS;

    class Script
    {
    public:
        void Initialize(GameObject gameObject)
        {
            _gameObject = gameObject;
            OnInitialize();
        }

        // Lifecycle
        virtual void OnInitialize() {}
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
        GameObject GetGameObject() const { return _gameObject; }
        entt::entity GetEntity() const { return _gameObject.GetHandle(); }
        Scene* GetScene() const { return _gameObject.GetScene(); }

    protected:
        GameObject _gameObject;
    };
} // namespace Frost