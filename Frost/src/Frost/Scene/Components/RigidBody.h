#pragma once

#include "Frost/Scene/ECS/ECS.h"
#include "Frost/Scene/ECS/Component.h"
#include "Frost/Scene/Components/WorldTransform.h"

// Jolt includes
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/MotionType.h>

namespace Frost
{
	class RigidBody : public Component
	{
	public:
		using PhysicLayers = JPH::ObjectLayer;

		//TODO : generaliser l'acces au gameObject.id depuis les component ? s'inspirer des scripts peut etre...
		RigidBody(GameObject::Id id, JPH::EMotionType motionType, PhysicLayers physicLayer, JPH::ShapeRefC shape);

		JPH::BodyID bodyId;
		int shapeId;
		JPH::EMotionType motionType;
		JPH::ObjectLayer objectLayer;
	};
}