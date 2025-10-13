#include "WorldTransformSystem.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/GameObjectInfo.h"

namespace Frost
{
	WorldTransformSystem::WorldTransformSystem()
	{
	}

	void WorldTransformSystem::Update(Frost::ECS& ecs, float deltaTime)
	{
		const auto& transforms = ecs.GetDataArray<Transform>();
		const auto& transformEntities = ecs.GetIndexMap<Transform>();

		for (size_t i = 0; i < transforms.size(); ++i)
		{
			GameObject::Id id = transformEntities[i];
			const Transform& transform = transforms[i];

			GameObjectInfo* info = ecs.GetComponent<GameObjectInfo>(id);
			WorldTransform* worldTransform = ecs.GetComponent<WorldTransform>(id);
			
			if (!worldTransform)
			{
				continue;
			}

			// Check if parent exists
			if (info && info->parentId != GameObject::InvalidId)
			{
				WorldTransform* parentTransform = ecs.GetComponent<WorldTransform>(info->parentId);
				if (parentTransform)
				{
					worldTransform->position = {
						parentTransform->position.x + transform.position.x,
						parentTransform->position.y + transform.position.y,
						parentTransform->position.z + transform.position.z
					};
					worldTransform->rotation = {
						parentTransform->rotation.x + transform.rotation.x,
						parentTransform->rotation.y + transform.rotation.y,
						parentTransform->rotation.z + transform.rotation.z
					};
					worldTransform->scale = {
						parentTransform->scale.x * transform.scale.x,
						parentTransform->scale.y * transform.scale.y,
						parentTransform->scale.z * transform.scale.z
					};
					continue;
				}
			}

			worldTransform->position = transform.position;
			worldTransform->rotation = transform.rotation;
			worldTransform->scale = transform.scale;
		}
	}
}