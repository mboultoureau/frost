#pragma once

#include "Frost/Scene/ECS/System.h"
#include "Frost/Scene/Scene.h"

namespace Frost
{
    class ScriptableSystem : public System
    {
    public:
        void OnAttach(Scene& scene) override;
        void OnDetach(Scene& scene) override;
        void Update(Scene& scene, float deltaTime) override;
        void PreFixedUpdate(Scene& scene, float deltaTime) override;
        void FixedUpdate(Scene& scene, float fixedDeltaTime) override;
        void LateUpdate(Scene& scene, float deltaTime) override;

        void OnScriptsWillReload();
        void OnScriptsReloaded();

    private:
        void _OnCreateScriptable(entt::registry& registry, entt::entity entity);
        void _OnDestroyScriptable(entt::registry& registry, entt::entity entity);

    private:
        Scene* _scene = nullptr;
    };
} // namespace Frost
