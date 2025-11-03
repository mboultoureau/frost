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
					DirectX::XMVECTOR parentRotation = DirectX::XMLoadFloat4(&parentTransform->rotation);

					// 2. Load the local Transform's Position (Vector)
					DirectX::XMVECTOR localPosition = DirectX::XMLoadFloat3(&localTransform.position);

					// 3. Rotate the local position by the parent's world rotation
					// This is the correct way to transform a child's position into world space relative to its parent.
					DirectX::XMVECTOR rotatedPosition = DirectX::XMVector3Rotate(localPosition, parentRotation);

					// 4. Calculate World Position: parent.position + (rotated)local.position
					DirectX::XMVECTOR parentPosition = DirectX::XMLoadFloat3(&parentTransform->position);
					DirectX::XMVECTOR worldPosition = DirectX::XMVectorAdd(parentPosition, rotatedPosition);

					// 5. Calculate World Rotation: parent.rotation * local.rotation (Quaternion Multiplication)
					// Note: Order matters! parent * local is standard for object hierarchies.
					DirectX::XMVECTOR localRotation = DirectX::XMLoadFloat4(&localTransform.rotation);
					DirectX::XMVECTOR worldRotation = DirectX::XMQuaternionMultiply(parentRotation, localRotation);

					// 6. Calculate World Scale: parent.scale * local.scale (Component-wise Multiplication)
					DirectX::XMVECTOR parentScale = DirectX::XMLoadFloat3(&parentTransform->scale);
					DirectX::XMVECTOR localScale = DirectX::XMLoadFloat3(&localTransform.scale);
					DirectX::XMVECTOR worldScale = DirectX::XMVectorMultiply(parentScale, localScale);

					// 7. Store the results back into the WorldTransform component
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