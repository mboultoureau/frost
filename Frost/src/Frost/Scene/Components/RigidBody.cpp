#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Physics/Physics.h"

Frost::RigidBody::RigidBody(GameObject::Id id, JPH::EMotionType motionType, PhysicLayers physicLayer, JPH::ShapeRefC shape) :
motionType{ motionType }, objectLayer{ physicLayer }, bodyId(), shapeId(-1)
{
	shapeId = Frost::Physics::Get().SetShapeToRigidbody(id, shape);
}
