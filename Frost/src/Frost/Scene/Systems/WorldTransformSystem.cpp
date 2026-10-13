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
        SynchronizeWorldTransforms(scene);
    }

    void WorldTransformSystem::Update(Scene& scene, float deltaTime)
    {
        SynchronizeWorldTransforms(scene);
    }

    void WorldTransformSystem::LateUpdate(Scene& scene, float deltaTime)
    {
        SynchronizeWorldTransforms(scene);
    }

    void WorldTransformSystem::SynchronizeWorldTransforms(Scene& scene)
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
                    _UpdateHierarchy(registry, entity, rootPos, rootRot, rootScale, false);
                }
            });

        auto flatView = registry.view<Transform, WorldTransform>(entt::exclude<Relationship>);
        flatView.each(
            [&](Transform& local, WorldTransform& world)
            {
                bool localDirty = local.isDirty || (local.position != world._lastLocalPosition) ||
                                  (local.rotation != world._lastLocalRotation) ||
                                  (local.scale != world._lastLocalScale);

                if (localDirty)
                {
                    world.position = local.position;
                    world.rotation = local.rotation;
                    world.scale = local.scale;

                    world._lastLocalPosition = local.position;
                    world._lastLocalRotation = local.rotation;
                    world._lastLocalScale = local.scale;

                    local.isDirty = false;
                }
            });
    }

    void WorldTransformSystem::_UpdateHierarchy(entt::registry& registry,
                                                entt::entity entity,
                                                DirectX::XMVECTOR parentPosition,
                                                DirectX::XMVECTOR parentRotation,
                                                DirectX::XMVECTOR parentScale,
                                                bool parentChanged)
    {
        auto* localTransform = registry.try_get<Transform>(entity);
        auto* worldTransform = registry.try_get<WorldTransform>(entity);
        auto* relationship = registry.try_get<Relationship>(entity);

        if (!localTransform || !worldTransform)
            return;

        bool localDirty = localTransform->isDirty || (localTransform->position != worldTransform->_lastLocalPosition) ||
                          (localTransform->rotation != worldTransform->_lastLocalRotation) ||
                          (localTransform->scale != worldTransform->_lastLocalScale);

        bool transformChanged = parentChanged || localDirty;

        using namespace DirectX;

        XMVECTOR newWorldPosition;
        XMVECTOR newWorldRotation;
        XMVECTOR newWorldScale;

        if (transformChanged)
        {
            if (relationship && relationship->parent == entt::null)
            {
                worldTransform->position = localTransform->position;
                worldTransform->rotation = localTransform->rotation;
                worldTransform->scale = localTransform->scale;

                newWorldPosition = Math::vector_cast<XMVECTOR>(worldTransform->position);
                newWorldRotation = Math::vector_cast<XMVECTOR>(worldTransform->rotation);
                newWorldScale = Math::vector_cast<XMVECTOR>(worldTransform->scale);
            }
            else
            {
                XMVECTOR localPosition = Math::vector_cast<XMVECTOR>(localTransform->position);
                XMVECTOR localRotation = Math::vector_cast<XMVECTOR>(localTransform->rotation);
                XMVECTOR localScale = Math::vector_cast<XMVECTOR>(localTransform->scale);

                newWorldScale = XMVectorMultiply(parentScale, localScale);
                newWorldRotation = XMQuaternionMultiply(localRotation, parentRotation);

                XMVECTOR scaledLocalPosition = XMVectorMultiply(localPosition, parentScale);
                XMVECTOR rotatedLocalPosition = XMVector3Rotate(scaledLocalPosition, parentRotation);
                newWorldPosition = XMVectorAdd(parentPosition, rotatedLocalPosition);

                worldTransform->position = Math::vector_cast<Math::Vector3>(newWorldPosition);
                worldTransform->rotation = Math::vector_cast<Math::Vector4>(newWorldRotation);
                worldTransform->scale = Math::vector_cast<Math::Vector3>(newWorldScale);
            }

            worldTransform->_lastLocalPosition = localTransform->position;
            worldTransform->_lastLocalRotation = localTransform->rotation;
            worldTransform->_lastLocalScale = localTransform->scale;
            localTransform->isDirty = false;
        }
        else
        {
            newWorldPosition = Math::vector_cast<XMVECTOR>(worldTransform->position);
            newWorldRotation = Math::vector_cast<XMVECTOR>(worldTransform->rotation);
            newWorldScale = Math::vector_cast<XMVECTOR>(worldTransform->scale);
        }

        if (relationship && relationship->firstChild != entt::null)
        {
            entt::entity currentChild = relationship->firstChild;
            while (currentChild != entt::null)
            {
                // Update children
                _UpdateHierarchy(
                    registry, currentChild, newWorldPosition, newWorldRotation, newWorldScale, transformChanged);

                // Update sibling
                auto& childRel = registry.get<Relationship>(currentChild);
                currentChild = childRel.nextSibling;
            }
        }
    }
} // namespace Frost