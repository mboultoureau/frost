#include "ScriptableSystem.h"

#include "Frost/Scene/ECS/ECS.h"
#include "Frost/Scene/Components/Scriptable.h"

namespace Frost
{
	void ScriptableSystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		auto& scriptables = ecs.GetDataArray<Scriptable>();
		for (size_t i = 0; i < scriptables.size(); i++)
		{
			auto& scriptable = scriptables[i];
			auto& indexMap = ecs.GetIndexMap<Scriptable>();
			auto gameObjectId = indexMap[i];
			for (auto& script : scriptable._scripts)
			{
				if (script->GetECS() == nullptr)
				{
					script->Initialize(gameObjectId, &ecs);
				}
				script->OnUpdate(deltaTime);
			}
		}
	}

	void ScriptableSystem::FixedUpdate(Frost::ECS& ecs, float fixedDeltaTime)
	{
		auto& scriptables = ecs.GetDataArray<Scriptable>();
		for (size_t i = 0; i < scriptables.size(); i++)
		{
			auto& scriptable = scriptables[i];
			auto& indexMap = ecs.GetIndexMap<Scriptable>();
			auto gameObjectId = indexMap[i];
			for (auto& script : scriptable._scripts)
			{
				if (script->GetECS() == nullptr)
				{
					script->Initialize(gameObjectId, &ecs);
				}
				script->OnFixedUpdate(fixedDeltaTime);
			}
		}
	}

	void ScriptableSystem::LateUpdate(Frost::ECS& ecs, float deltaTime)
	{
		auto& scriptables = ecs.GetDataArray<Scriptable>();
		for (size_t i = 0; i < scriptables.size(); i++)
		{
			auto& scriptable = scriptables[i];
			auto& indexMap = ecs.GetIndexMap<Scriptable>();
			auto gameObjectId = indexMap[i];
			for (auto& script : scriptable._scripts)
			{
				if (script->GetECS() == nullptr)
				{
					script->Initialize(gameObjectId, &ecs);
				}
				script->OnLateUpdate(deltaTime);
			}
		}
	}
}