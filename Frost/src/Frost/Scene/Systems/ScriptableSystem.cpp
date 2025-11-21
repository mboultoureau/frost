#include "Frost/Scene/Systems/ScriptableSystem.h"
#include "Frost/Scene/Components/Scriptable.h"
#include "Frost/Scene/Components/Script.h"

using namespace Frost::Component;

namespace Frost
{
	void ScriptableSystem::Update(Scene& scene, float deltaTime)
	{
		auto view = scene.ViewActive<Scriptable>();

		for (auto entity : view)
		{
			auto& scriptable = view.get<Scriptable>(entity);
			GameObject gameObject(entity, &scene);

			for (auto& script : scriptable._scripts)
			{
				if (!script->GetGameObject())
				{
					script->Initialize(gameObject);
				}

				script->OnUpdate(deltaTime);
			}
		}
	}

	void ScriptableSystem::PreFixedUpdate(Scene& scene, float fixedDeltaTime)
	{
		auto view = scene.ViewActive<Scriptable>();

		for (auto entity : view)
		{
			auto& scriptable = view.get<Scriptable>(entity);
			GameObject gameObject(entity, &scene);

			for (auto& script : scriptable._scripts)
			{
				if (!script->GetGameObject())
				{
					script->Initialize(gameObject);
				}

				script->OnPreFixedUpdate(fixedDeltaTime);
			}
		}
	}

	void ScriptableSystem::FixedUpdate(Scene& scene, float fixedDeltaTime)
	{
		auto view = scene.ViewActive<Scriptable>();

		for (auto entity : view)
		{
			auto& scriptable = view.get<Scriptable>(entity);
			GameObject gameObject(entity, &scene);

			for (auto& script : scriptable._scripts)
			{
				if (!script->GetGameObject())
				{
					script->Initialize(gameObject);
				}

				script->OnFixedUpdate(fixedDeltaTime);
			}
		}
	}

	void ScriptableSystem::LateUpdate(Scene& scene, float deltaTime)
	{
		auto view = scene.ViewActive<Scriptable>();

		for (auto entity : view)
		{
			auto& scriptable = view.get<Scriptable>(entity);
			GameObject gameObject(entity, &scene);

			for (auto& script : scriptable._scripts)
			{
				if (!script->GetGameObject())
				{
					script->Initialize(gameObject);
				}

				script->OnLateUpdate(deltaTime);
			}
		}
	}
}