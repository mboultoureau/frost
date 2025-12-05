#pragma once

#include "Frost/Scene/ECS/Component.h"
#include "Frost/Utils/Math/Vector.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include <variant>

namespace Frost::Component
{
    enum class CollisionShapeType
    {
        Box = 0,
        Sphere,
        Capsule,
        Cylinder
    };

    struct ShapeBox
    {
        Math::Vector3 halfExtent = { 0.5f, 0.5f, 0.5f };
        float convexRadius = 0.05f;
    };

    struct ShapeSphere
    {
        float radius = 0.5f;
    };

    struct ShapeCapsule
    {
        float halfHeight = 0.5f;
        float radius = 0.5f;
    };

    struct ShapeCylinder
    {
        float halfHeight = 0.5f;
        float radius = 0.5f;
        float convexRadius = 0.05f;
    };

    using CollisionShapeConfig = std::variant<ShapeBox, ShapeSphere, ShapeCapsule, ShapeCylinder>;

    struct RigidBody : public Component
    {
        enum class MotionType
        {
            Static,
            Kinematic,
            Dynamic
        };
        MotionType motionType = MotionType::Static;

        bool isSensor = false;
        bool allowSleeping = true;
        float friction = 0.2f;
        float restitution = 0.0f;

        enum class OverrideMassProperties
        {
            CalculateMassAndInertia,
            CalculateInertia,
            MassAndInertiaProvided
        };
        OverrideMassProperties overrideMassProperties = OverrideMassProperties::CalculateMassAndInertia;
        float mass = 1.0f;

        float linearDamping = 0.05f;
        float angularDamping = 0.05f;

        float gravityFactor = 1.0f;

        CollisionShapeConfig shape = ShapeBox{};

        bool lockPositionX = false;
        bool lockPositionY = false;
        bool lockPositionZ = false;
        bool lockRotationX = false;
        bool lockRotationY = false;
        bool lockRotationZ = false;

        JPH::BodyID runtimeBodyID;
        JPH::ObjectLayer objectLayer;

        RigidBody() : runtimeBodyID{}, objectLayer{} {}

        RigidBody(JPH::BodyID bodyID) : runtimeBodyID{ bodyID } {}

        RigidBody(CollisionShapeConfig shapeConfig, JPH::ObjectLayer objectLayer) :
            shape{ shapeConfig }, runtimeBodyID{}, objectLayer{ objectLayer }
        {
        }

        RigidBody(CollisionShapeConfig shapeConfig, JPH::ObjectLayer objectLayer, MotionType motionType) :
            shape{ shapeConfig }, runtimeBodyID{}, objectLayer{ objectLayer }, motionType{ motionType }
        {
        }
    };
} // namespace Frost::Component