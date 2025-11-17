#include "WorldTransformSystem.h"

#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Scene/Components/Meta.h"

using namespace Frost::Component;

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

			Meta* info = ecs.GetComponent<Meta>(id);
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
					DirectX::XMVECTOR parentRotation = vector_cast<DirectX::XMVECTOR>(parentTransform->rotation);
					DirectX::XMVECTOR localPosition = vector_cast<DirectX::XMVECTOR>(localTransform.position);
					DirectX::XMVECTOR rotatedPosition = DirectX::XMVector3Rotate(localPosition, parentRotation);

					DirectX::XMVECTOR parentPosition = vector_cast<DirectX::XMVECTOR>(parentTransform->position);
					DirectX::XMVECTOR worldPosition = DirectX::XMVectorAdd(parentPosition, rotatedPosition);

					DirectX::XMVECTOR localRotation = vector_cast<DirectX::XMVECTOR>(localTransform.rotation);
					DirectX::XMVECTOR worldRotation = DirectX::XMQuaternionMultiply(localRotation, parentRotation);

					DirectX::XMVECTOR parentScale = vector_cast<DirectX::XMVECTOR>(parentTransform->scale);
					DirectX::XMVECTOR localScale = vector_cast<DirectX::XMVECTOR>(localTransform.scale);
					DirectX::XMVECTOR worldScale = DirectX::XMVectorMultiply(parentScale, localScale);

					worldTransform->position = Math::vector_cast<Math::Vector3>(worldPosition);
					worldTransform->rotation = Math::vector_cast<Math::Vector4>(worldRotation);
					worldTransform->scale = Math::vector_cast<Math::Vector3>(worldScale);

					continue;
				}
			}

			worldTransform->position = localTransform.position;
			worldTransform->rotation = localTransform.rotation;
			worldTransform->scale = localTransform.scale;
		}
	}
}