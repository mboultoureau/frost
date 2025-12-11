#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/Relationship.h"
#include "Frost/Physics/Physics.h"
#include "Frost/Scripting/Script.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Transform.h"
#include "Frost/Utils/Math/Matrix.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Frost::Component;

namespace Frost
{
    static JPH::Ref<JPH::Shape> CreateJoltShape(const Component::CollisionShapeConfig& config,
                                                const Math::Vector3& scale)
    {
        JPH::ShapeSettings::ShapeResult result;

        JPH::Vec3 s = { abs(scale.x), abs(scale.y), abs(scale.z) };

        float minScale = 0.001f;
        if (s.GetX() < minScale)
            s.SetX(minScale);
        if (s.GetY() < minScale)
            s.SetY(minScale);
        if (s.GetZ() < minScale)
            s.SetZ(minScale);

        if (std::holds_alternative<ShapeBox>(config))
        {
            const auto& box = std::get<ShapeBox>(config);
            result = JPH::BoxShapeSettings({ box.halfExtent.x, box.halfExtent.y, box.halfExtent.z }, box.convexRadius)
                         .Create();
        }
        else if (std::holds_alternative<ShapeSphere>(config))
        {
            const auto& sphere = std::get<ShapeSphere>(config);
            result = JPH::SphereShapeSettings(sphere.radius).Create();
        }
        else if (std::holds_alternative<ShapeCapsule>(config))
        {
            const auto& capsule = std::get<ShapeCapsule>(config);
            result = JPH::CapsuleShapeSettings(capsule.halfHeight, capsule.radius).Create();
        }
        else if (std::holds_alternative<ShapeCylinder>(config))
        {
            const auto& cylinder = std::get<ShapeCylinder>(config);
            result = JPH::CylinderShapeSettings(cylinder.halfHeight, cylinder.radius, cylinder.convexRadius).Create();
        }
        else if (std::holds_alternative<ShapeMesh>(config))
        {
            const auto& meshConfig = std::get<ShapeMesh>(config);

            if (meshConfig.path.empty())
            {
                result = JPH::BoxShapeSettings({ 0.5f, 0.5f, 0.5f }, 0.05f).Create();
            }
            else
            {
                Assimp::Importer importer;
                const aiScene* scene = importer.ReadFile(
                    meshConfig.path.c_str(),
                    aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenSmoothNormals |
                        aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials | aiProcess_PreTransformVertices);

                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
                {
                    result = JPH::BoxShapeSettings({ 0.5f, 0.5f, 0.5f }, 0.05f).Create();
                }

                else
                {
                    JPH::VertexList vertices;
                    JPH::IndexedTriangleList triangles;

                    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
                    {
                        const aiMesh* mesh = scene->mMeshes[i];

                        uint32_t vertexStartIdx = (uint32_t)vertices.size();

                        for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
                        {
                            const aiVector3D& pos = mesh->mVertices[v];
                            vertices.push_back({ pos.x, pos.y, pos.z });
                        }

                        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
                        {
                            const aiFace& face = mesh->mFaces[f];
                            if (face.mNumIndices == 3)
                            {
                                JPH::IndexedTriangle triangle;
                                triangle.mIdx[0] = vertexStartIdx + face.mIndices[0];
                                triangle.mIdx[1] = vertexStartIdx + face.mIndices[1];
                                triangle.mIdx[2] = vertexStartIdx + face.mIndices[2];
                                triangle.mMaterialIndex = 0;
                                triangles.push_back(triangle);
                            }
                        }
                    }

                    if (vertices.empty() || triangles.empty())
                    {
                        result = JPH::BoxShapeSettings({ 0.5f, 0.5f, 0.5f }, 0.05f).Create();
                    }
                    else
                    {
                        JPH::MeshShapeSettings settings(vertices, triangles);
                        result = settings.Create();

                        if (result.HasError())
                        {
                            result = JPH::BoxShapeSettings({ 0.5f, 0.5f, 0.5f }, 0.05f).Create();
                        }
                    }
                }
            }
        }
        else
        {
            result = JPH::BoxShapeSettings({ 0.5f, 0.5f, 0.5f }, 0.05f).Create();
        }

        if (result.HasError())
        {
            return nullptr;
        }

        if (s.IsClose(JPH::Vec3::sReplicate(1.0f), 1.0e-5f))
        {
            return result.Get();
        }
        else
        {
            JPH::ScaledShapeSettings scaledShapeSettings(result.Get(), s);
            JPH::ShapeSettings::ShapeResult scaledResult = scaledShapeSettings.Create();
            if (scaledResult.HasError())
            {
                return nullptr;
            }
            return scaledResult.Get();
        }
    }

    void PhysicSystem::OnAttach(Scene& scene)
    {
        _scene = &scene;

        auto& registry = scene.GetRegistry();
        // registry.on_destroy<Component::RigidBody>().connect<&PhysicSystem::_OnDestroyBody>(*this);

        auto view = scene.GetRegistry().view<RigidBody>();
        for (auto entity : view)
        {
            _CreateBodyForEntity(scene, entity);
        }
    }

    void PhysicSystem::OnDetach(Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        // registry.on_destroy<Component::RigidBody>().disconnect<&PhysicSystem::_OnDestroyBody>(*this);

        auto view = scene.GetRegistry().view<RigidBody>();
        for (auto entity : view)
        {
            _DestroyBodyForEntity(scene, entity);
        }
    }

    void PhysicSystem::_OnDestroyBody(entt::registry& registry, entt::entity entity)
    {
        _DestroyBodyForEntity(*_scene, entity);
    }

    void PhysicSystem::FixedUpdate(Scene& scene, float fixedDeltaTime)
    {
        {
            auto view = scene.GetRegistry().view<RigidBody, WorldTransform>();
            view.each(
                [&](entt::entity entity, RigidBody& rb, WorldTransform& worldTransform)
                {
                    if (rb.runtimeBodyID.IsInvalid())
                    {
                        _CreateBodyForEntity(scene, entity);
                    }
                });
        }

        Physics::Get().UpdatePhysics(fixedDeltaTime);

        _SynchronizeTransforms(scene);

        _HandleAwakeVector(scene, fixedDeltaTime);
        _HandleSleepVector(scene, fixedDeltaTime);
        _HandleOnCollisionEnterVector(scene, fixedDeltaTime);
        _HandleOnCollisionStayVector(scene, fixedDeltaTime);
        _HandleOnCollisionRemovedVector(scene, fixedDeltaTime);
    }

    void PhysicSystem::LateUpdate(Scene& scene, float deltaTime)
    {
#ifdef FT_DEBUG
        Physics::Get().DrawDebug();
#endif
    }

    void PhysicSystem::NotifyRigidBodyUpdate(Scene& scene, GameObject entity)
    {
        if (!scene.GetRegistry().valid(entity.GetHandle()))
            return;

        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody, Component::Transform, Component::WorldTransform>(entity.GetHandle()))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity.GetHandle());

        if (!rb.runtimeBodyID.IsInvalid())
        {
            _DestroyBodyForEntity(scene, entity.GetHandle());
        }

        _CreateBodyForEntity(scene, entity.GetHandle());

        /*
        if (rb.runtimeBodyID.IsInvalid())
        {
            _CreateBodyForEntity(scene, entity.GetHandle());
            return;
        }


        JPH::BodyInterface& bodyInterface = Physics::GetBodyInterface();

        bodyInterface.SetPositionAndRotation(rb.runtimeBodyID,
                                             Math::vector_cast<JPH::RVec3>(transform.position),
                                             Math::vector_cast<JPH::Quat>(transform.rotation),
                                             JPH::EActivation::DontActivate);

        JPH::Ref<JPH::Shape> newShape = CreateJoltShape(rb.shape, transform.scale);

        if (newShape)
        {
            bodyInterface.SetShape(rb.runtimeBodyID, newShape, true, JPH::EActivation::DontActivate);
        }

        bodyInterface.SetMotionType(
            rb.runtimeBodyID, static_cast<JPH::EMotionType>(rb.motionType), JPH::EActivation::DontActivate);
        bodyInterface.SetFriction(rb.runtimeBodyID, rb.friction);
        bodyInterface.SetRestitution(rb.runtimeBodyID, rb.restitution);
        */
    }

    void PhysicSystem::_CreateBodyForEntity(Scene& scene, entt::entity entity)
    {
        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody, Component::WorldTransform>(entity))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity);
        auto& worldTransform = registry.get<Component::WorldTransform>(entity);
        auto& body_interface = Physics::GetBodyInterface();

        if (!rb.runtimeBodyID.IsInvalid())
            return;

        JPH::Ref<JPH::Shape> finalShape = CreateJoltShape(rb.shape, worldTransform.scale);

        if (!finalShape)
        {
            FT_ENGINE_ERROR("PhysicSystem: Failed to create shape for entity {0}", (uint32_t)entity);
            return;
        }

        JPH::BodyCreationSettings bodySettings(finalShape,
                                               Math::vector_cast<JPH::RVec3>(worldTransform.position),
                                               Math::vector_cast<JPH::Quat>(worldTransform.rotation),
                                               static_cast<JPH::EMotionType>(rb.motionType),
                                               rb.objectLayer);

        bodySettings.mIsSensor = rb.isSensor;
        bodySettings.mAllowSleeping = rb.allowSleeping;
        bodySettings.mFriction = rb.friction;
        bodySettings.mRestitution = rb.restitution;
        bodySettings.mLinearDamping = rb.linearDamping;
        bodySettings.mAngularDamping = rb.angularDamping;
        bodySettings.mGravityFactor = rb.gravityFactor;

        JPH::EAllowedDOFs dofs = JPH::EAllowedDOFs::All;
        if (rb.lockPositionX)
            dofs &= ~JPH::EAllowedDOFs::TranslationX;
        if (rb.lockPositionY)
            dofs &= ~JPH::EAllowedDOFs::TranslationY;
        if (rb.lockPositionZ)
            dofs &= ~JPH::EAllowedDOFs::TranslationZ;
        if (rb.lockRotationX)
            dofs &= ~JPH::EAllowedDOFs::RotationX;
        if (rb.lockRotationY)
            dofs &= ~JPH::EAllowedDOFs::RotationY;
        if (rb.lockRotationZ)
            dofs &= ~JPH::EAllowedDOFs::RotationZ;
        bodySettings.mAllowedDOFs = dofs;

        if (rb.motionType == Component::RigidBody::MotionType::Dynamic)
        {
            bodySettings.mOverrideMassProperties = static_cast<JPH::EOverrideMassProperties>(rb.overrideMassProperties);
            if (rb.overrideMassProperties != Component::RigidBody::OverrideMassProperties::CalculateMassAndInertia)
                bodySettings.mMassPropertiesOverride.mMass = rb.mass;
        }

        bodySettings.mUserData = static_cast<uint64_t>(entity);

        JPH::Body* body = body_interface.CreateBody(bodySettings);
        if (body)
        {
            rb.runtimeBodyID = body->GetID();
            body_interface.AddBody(rb.runtimeBodyID, JPH::EActivation::Activate);
        }
        else
        {
            FT_ENGINE_ERROR("PhysicSystem: Failed to create body for entity {0}", (uint32_t)entity);
        }
    }

    void PhysicSystem::_DestroyBodyForEntity(Scene& scene, entt::entity entity)
    {
        auto& registry = scene.GetRegistry();
        if (!registry.all_of<Component::RigidBody>(entity))
            return;

        auto& rb = registry.get<Component::RigidBody>(entity);
        if (rb.runtimeBodyID.IsInvalid())
            return;

        auto& body_interface = Physics::GetBodyInterface();
        body_interface.RemoveBody(rb.runtimeBodyID);
        body_interface.DestroyBody(rb.runtimeBodyID);
        rb.runtimeBodyID = JPH::BodyID();
    }

    void PhysicSystem::_SynchronizeTransforms(Scene& scene)
    {
        auto& registry = scene.GetRegistry();
        auto& body_interface = Physics::GetBodyInterface();

        auto view = registry.view<Component::RigidBody, Component::Transform, Component::WorldTransform>();

        view.each(
            [&](auto entity,
                Component::RigidBody& rb,
                Component::Transform& localTransform,
                Component::WorldTransform& worldTransform)
            {
                if (rb.runtimeBodyID.IsInvalid() || !body_interface.IsActive(rb.runtimeBodyID))
                    return;

                auto jBodyPos = body_interface.GetPosition(rb.runtimeBodyID);
                auto jBodyRot = body_interface.GetRotation(rb.runtimeBodyID);

                Math::Vector3 newWorldPosition = Math::vector_cast<Math::Vector3>(jBodyPos);
                Math::Vector4 newWorldRotation = { jBodyRot.GetX(), jBodyRot.GetY(), jBodyRot.GetZ(), jBodyRot.GetW() };

                auto* relationship = registry.try_get<Component::Relationship>(entity);
                if (relationship && relationship->parent != entt::null)
                {
                    auto* parentWorldTransform = registry.try_get<Component::WorldTransform>(relationship->parent);
                    if (parentWorldTransform)
                    {
                        Math::Matrix4x4 newWorldMat = Math::Matrix4x4::CreateFromQuaternion(newWorldRotation) *
                                                      Math::Matrix4x4::CreateTranslation(newWorldPosition);

                        Math::Matrix4x4 parentWorldMat = Math::GetTransformMatrix(*parentWorldTransform);

                        DirectX::XMMATRIX parentMatDX = Math::LoadMatrix(parentWorldMat);
                        DirectX::XMVECTOR det;
                        DirectX::XMMATRIX parentInverseMatDX = DirectX::XMMatrixInverse(&det, parentMatDX);

                        Math::Matrix4x4 parentInverseMat;
                        Math::StoreMatrix(&parentInverseMat, parentInverseMatDX);

                        Math::Matrix4x4 newLocalMat = newWorldMat * parentInverseMat;

                        Math::Vector3 newLocalPosition, newLocalScale;
                        Math::Vector4 newLocalRotation;
                        Math::DecomposeTransform(newLocalMat, newLocalPosition, newLocalRotation, newLocalScale);

                        localTransform.position = newLocalPosition;
                        localTransform.rotation = newLocalRotation;
                    }
                }
                else
                {
                    localTransform.position = newWorldPosition;
                    localTransform.rotation = newWorldRotation;
                }
            });
    }

    void Frost::PhysicSystem::_HandleAwakeVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnAwake)
        {
            entt::entity entity = Physics::GetEntityID(params.inBodyID);
            _ExecuteOnScripts(scene, entity, [&](Scripting::Script* script) { script->OnAwake(deltaTime); });
        }
        physics.bodiesOnAwake.clear();
    }

    void Frost::PhysicSystem::_HandleSleepVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnSleep)
        {
            entt::entity entity = Physics::GetEntityID(params.inBodyID);
            _ExecuteOnScripts(scene, entity, [&](Scripting::Script* script) { script->OnSleep(deltaTime); });
        }
        physics.bodiesOnSleep.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionEnterVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();

        for (const auto& params : physics.bodiesOnCollisionEnter)
        {
            entt::entity entity1 = Physics::GetEntityID(params.inBody1.GetID());
            entt::entity entity2 = Physics::GetEntityID(params.inBody2.GetID());

            _ExecuteOnScripts(
                scene, entity1, [&](Scripting::Script* script) { script->OnCollisionEnter(params, deltaTime); });

            _ExecuteOnScripts(
                scene, entity2, [&](Scripting::Script* script) { script->OnCollisionEnter(params, deltaTime); });
        }
        physics.bodiesOnCollisionEnter.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionStayVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();
        for (const auto& params : physics.bodiesOnCollisionStay)
        {
            entt::entity entity1 = Physics::GetEntityID(params.inBody1.GetID());
            entt::entity entity2 = Physics::GetEntityID(params.inBody2.GetID());

            _ExecuteOnScripts(
                scene, entity1, [&](Scripting::Script* script) { script->OnCollisionStay(params, deltaTime); });

            _ExecuteOnScripts(
                scene, entity2, [&](Scripting::Script* script) { script->OnCollisionStay(params, deltaTime); });

            physics.currentFrameBodyIDsOnCollisionStay.emplace(entity1, entity2);
        }
        physics.bodiesOnCollisionStay.clear();
    }

    void Frost::PhysicSystem::_HandleOnCollisionRemovedVector(Scene& scene, float deltaTime)
    {
        auto& physics = Physics::Get();

        for (const auto& pair : physics.lastFrameBodyIDsOnCollisionStay)
        {
            if (physics.currentFrameBodyIDsOnCollisionStay.find(pair) ==
                physics.currentFrameBodyIDsOnCollisionStay.end())
            {
                entt::entity entity1 = pair.first;
                entt::entity entity2 = pair.second;

                std::pair<entt::entity, entt::entity> exitParams = { entity1, entity2 };

                _ExecuteOnScripts(
                    scene, entity1, [&](Scripting::Script* script) { script->OnCollisionExit(exitParams, deltaTime); });

                _ExecuteOnScripts(
                    scene, entity2, [&](Scripting::Script* script) { script->OnCollisionExit(exitParams, deltaTime); });
            }
        }

        // Swap buffers for next frame
        physics.lastFrameBodyIDsOnCollisionStay = physics.currentFrameBodyIDsOnCollisionStay;
        physics.currentFrameBodyIDsOnCollisionStay.clear();
    }

} // namespace Frost
