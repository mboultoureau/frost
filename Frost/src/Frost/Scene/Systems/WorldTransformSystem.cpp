#include "WorldTransformSystem.h"

#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"

using namespace DirectX;
using namespace Frost::Component;

namespace Frost
{
    WorldTransformSystem::WorldTransformSystem() {}

    void WorldTransformSystem::PreFixedUpdate(Scene& scene, float deltaTime)
    {
        auto& registry = scene.GetRegistry();
        auto view = scene.ViewActive<Transform, WorldTransform, Relationship>();

        XMVECTOR rootPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        XMVECTOR rootRot = XMQuaternionIdentity();
        XMVECTOR rootScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);

        view.each(
            [&](entt::entity entity, const Transform&, const WorldTransform&, const Relationship& relationship)
            {
                if (relationship.parent == entt::null)
                {
                    _UpdateHierarchy(registry, entity, rootPos, rootRot, rootScale);
                }
            });

        auto flatView = registry.view<Transform, WorldTransform>(entt::exclude<Relationship>);
        flatView.each(
            [&](const Transform& local, WorldTransform& world)
            {
                world.position = local.position;
                world.rotation = local.rotation;
                world.scale = local.scale;
            });
    }

    void WorldTransformSystem::_UpdateHierarchy(entt::registry& registry,
                                                entt::entity entity,
                                                DirectX::XMVECTOR parentPosition,
                                                DirectX::XMVECTOR parentRotation,
                                                DirectX::XMVECTOR parentScale)
    {
        auto* localTransform = registry.try_get<Transform>(entity);
        auto* worldTransform = registry.try_get<WorldTransform>(entity);
        auto* relationship = registry.try_get<Relationship>(entity);

        if (!localTransform || !worldTransform)
            return;

        using namespace DirectX;

        XMVECTOR localPosition = Math::vector_cast<XMVECTOR>(localTransform->position);
        XMVECTOR localRotation = Math::vector_cast<XMVECTOR>(localTransform->rotation);
        XMVECTOR localScale = Math::vector_cast<XMVECTOR>(localTransform->scale);

        XMVECTOR newWorldScale = XMVectorMultiply(parentScale, localScale);
        XMVECTOR newWorldRotation = XMQuaternionMultiply(localRotation, parentRotation);

        XMVECTOR scaledLocalPosition = XMVectorMultiply(localPosition, parentScale);
        XMVECTOR rotatedLocalPosition = XMVector3Rotate(scaledLocalPosition, parentRotation);
        XMVECTOR newWorldPosition = XMVectorAdd(parentPosition, rotatedLocalPosition);

        worldTransform->position = Math::vector_cast<Math::Vector3>(newWorldPosition);
        worldTransform->rotation = Math::vector_cast<Math::Vector4>(newWorldRotation);
        worldTransform->scale = Math::vector_cast<Math::Vector3>(newWorldScale);

        if (relationship && relationship->firstChild != entt::null)
        {
            entt::entity currentChild = relationship->firstChild;
            while (currentChild != entt::null)
            {
                // Update children
                _UpdateHierarchy(registry, currentChild, newWorldPosition, newWorldRotation, newWorldScale);

                // Update sibling
                auto& childRel = registry.get<Relationship>(currentChild);
                currentChild = childRel.nextSibling;
            }
        }
    }
} // namespace Frost