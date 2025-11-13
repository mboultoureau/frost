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
			const Transform& localTransform = transforms[i];

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
					//Get info
					DirectX::XMVECTOR parentPosition = DirectX::XMLoadFloat3(&parentTransform->position);
					DirectX::XMVECTOR parentRotation = DirectX::XMLoadFloat4(&parentTransform->rotation);

					DirectX::XMVECTOR localPosition = DirectX::XMLoadFloat3(&localTransform.position);
					DirectX::XMVECTOR localRotation = DirectX::XMLoadFloat4(&localTransform.rotation);


					// calc worldtransform pos
					DirectX::XMVECTOR rotatedPosition = DirectX::XMVector3Rotate(localPosition, parentRotation);
					DirectX::XMVECTOR worldPosition = DirectX::XMVectorAdd(parentPosition, rotatedPosition);

					// calc worldtransform rot
					DirectX::XMVECTOR worldRotation = DirectX::XMQuaternionMultiply(localRotation, parentRotation);

					// calc worldtransform rot
					DirectX::XMVECTOR parentScale = DirectX::XMLoadFloat3(&parentTransform->scale);
					DirectX::XMVECTOR localScale = DirectX::XMLoadFloat3(&localTransform.scale);
					DirectX::XMVECTOR worldScale = DirectX::XMVectorMultiply(parentScale, localScale);


					DirectX::XMStoreFloat3(&worldTransform->position, worldPosition);
					DirectX::XMStoreFloat4(&worldTransform->rotation, worldRotation); // Store as XMFLOAT4
					DirectX::XMStoreFloat3(&worldTransform->scale, worldScale);

					continue;
				}
			}

			worldTransform->position = localTransform.position;
			worldTransform->rotation = localTransform.rotation;
			worldTransform->scale = localTransform.scale;
		}
	}
}