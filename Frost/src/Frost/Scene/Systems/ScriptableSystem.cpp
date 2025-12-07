#include "Frost/Scene/Systems/ScriptableSystem.h"
#include "Frost/Scripting/Script.h"
#include "Frost/Scripting/ScriptingEngine.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Debugging/Logger.h"

using namespace Frost::Component;
using namespace Frost::Scripting;

namespace Frost
{
    void ScriptableSystem::OnAttach(Scene& scene)
    {
        _scene = &scene;

        auto& registry = scene.GetRegistry();

        registry.on_construct<Component::Scriptable>().connect<&ScriptableSystem::_OnCreateScriptable>(*this);
        registry.on_destroy<Component::Scriptable>().connect<&ScriptableSystem::_OnDestroyScriptable>(*this);
    }

    void ScriptableSystem::OnDetach(Scene& scene)
    {
        auto& registry = scene.GetRegistry();

        registry.on_construct<Component::Scriptable>().disconnect<&ScriptableSystem::_OnCreateScriptable>(*this);
        registry.on_destroy<Component::Scriptable>().disconnect<&ScriptableSystem::_OnDestroyScriptable>(*this);

        _scene = nullptr;
    }

    void ScriptableSystem::Update(Scene& scene, float deltaTime)
    {
        auto view = _scene->ViewActive<Scriptable>();

        for (auto entity : view)
        {
            auto& scriptable = view.get<Scriptable>(entity);
            GameObject gameObject(entity, _scene);

            for (auto& script : scriptable._scripts)
            {
                script->OnUpdate(deltaTime);
            }
        }
    }

    void ScriptableSystem::PreFixedUpdate(Scene& scene, float fixedDeltaTime)
    {
        auto view = _scene->ViewActive<Scriptable>();

        for (auto entity : view)
        {
            auto& scriptable = view.get<Scriptable>(entity);
            GameObject gameObject(entity, _scene);

            for (auto& script : scriptable._scripts)
            {
                script->OnPreFixedUpdate(fixedDeltaTime);
            }
        }
    }

    void ScriptableSystem::FixedUpdate(Scene& scene, float fixedDeltaTime)
    {
        auto view = _scene->ViewActive<Scriptable>();

        for (auto entity : view)
        {
            auto& scriptable = view.get<Scriptable>(entity);
            GameObject gameObject(entity, _scene);

            for (auto& script : scriptable._scripts)
            {
                script->OnFixedUpdate(fixedDeltaTime);
            }
        }
    }

    void ScriptableSystem::LateUpdate(Scene& scene, float deltaTime)
    {
        auto view = _scene->ViewActive<Scriptable>();

        for (auto entity : view)
        {
            auto& scriptable = view.get<Scriptable>(entity);
            GameObject gameObject(entity, _scene);

            for (auto& script : scriptable._scripts)
            {
                script->OnLateUpdate(deltaTime);
            }
        }
    }

    void ScriptableSystem::OnScriptsWillReload()
    {
        if (!_scene)
            return;

        auto view = _scene->GetRegistry().view<Scriptable>();
        for (auto entity : view)
        {
            auto& scriptable = view.get<Scriptable>(entity);
            _OnDestroyScriptable(_scene->GetRegistry(), entity);
            scriptable._scripts.clear();
        }
    }

    void ScriptableSystem::OnScriptsReloaded()
    {
        if (!_scene)
            return;

        auto view = _scene->GetRegistry().view<Scriptable>();
        for (auto entity : view)
        {
            _OnCreateScriptable(_scene->GetRegistry(), entity);
        }
    }

    void ScriptableSystem::_OnCreateScriptable(entt::registry& registry, entt::entity entity)
    {
        auto& scriptable = registry.get<Component::Scriptable>(entity);
        GameObject gameObject(entity, _scene);

        scriptable._scripts.clear();

        for (const auto& scriptName : scriptable.scriptNames)
        {
            Scripting::Script* script = ScriptingEngine::CreateScript(scriptName);
            if (script)
            {
                script->SetGameObject(gameObject);
                script->OnCreate();
                scriptable._scripts.emplace_back(script);
            }
            else
            {
                FT_ENGINE_ERROR(
                    "Failed to create script '{}' for entity {}", scriptName, static_cast<uint32_t>(entity));
            }
        }
    }

    void ScriptableSystem::_OnDestroyScriptable(entt::registry& registry, entt::entity entity)
    {
        auto& scriptable = registry.get<Component::Scriptable>(entity);
        for (auto& script : scriptable._scripts)
        {
            script->OnDestroy();
        }
    }
} // namespace Frost