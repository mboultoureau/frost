#include "Boat.h"
#include "../../../Physics/PhysicsLayer.h"
#include "../../../Game.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/WorldTransform.h"

#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>



// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include "../../Water.h"
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;


Boat::Boat(Player* player, RendererParameters params) : Vehicle(player, params)
{
    using namespace JPH;
    // Create motorcycle controller
    // Set playerbody to made body
    RenderMesh(false);
}

JPH::BodyID Boat::Appear()
{
    using namespace JPH;

    RenderMesh(true);

    // ----- Transform -----
    auto tr = _scene->GetComponent<Transform>(_player->GetPlayerID());
    Vec3 pos = Math::vector_cast<Vec3>(tr->position);
    Quat rot = Math::vector_cast<Quat>(tr->rotation);

    // Create boat
    ConvexHullShapeSettings boat_hull;
    boat_hull.mPoints = {
        Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
        Vec3(cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
        Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
        Vec3(cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
        Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
        Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
        Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
        Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
        Vec3(0, cHalfBoatHeight, cHalfBoatLength + cBoatBowLength)
    };
    boat_hull.SetEmbedded();
    OffsetCenterOfMassShapeSettings com_offset(Vec3(0, -cHalfBoatHeight, 0), &boat_hull);
    com_offset.SetEmbedded();
    BodyCreationSettings boat(&com_offset, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
    boat.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    boat.mMassPropertiesOverride.mMass = cBoatMass;
    boat.mFriction = cBoatFriction;
    _body = Physics::CreateBody(boat);
    _bodyId = _body->GetID();
    Physics::AddBody(_bodyId, EActivation::Activate);

    Boat::RegisterBoat(_bodyId, this);

    return _bodyId;
}



void Boat::Disappear()
{
    RenderMesh(false);

    Physics::RemoveAndDestroyBody(_bodyId);
    Boat::UnregisterBoat(_bodyId);
}


void Boat::ProcessBoatInput(float deltaTime) {

}



void Boat::OnPreFixedUpdate(float deltaTime)
{
    using namespace JPH;
    ProcessBoatInput(deltaTime);


    // On user input, assure that the boat is active
    if (_right != 0.0f || _forward != 0.0f)
        Physics::ActivateBody(_bodyId);

    // Apply forces to rear of boat where the propeller would be but only when the propeller is under water
    RVec3 propeller_position = _body->GetWorldTransform() * Vec3(0, -cHalfBoatHeight, -cHalfBoatLength);
    RVec3 bow_position = _body->GetWorldTransform() * Vec3(0, -cHalfBoatHeight, cHalfBoatLength);
    Vec3 forward = _body->GetRotation().RotateAxisZ();
    Vec3 realFwd = Vec3(forward.GetX(), 0, forward.GetZ());
    Vec3 right = _body->GetRotation().RotateAxisX();

//    auto angVel = _body->GetAngularVelocity();
    if (_currentWater) {
        //RVec3 propeller_surface_position = _currentWater->GetWaterSurfacePosition(propeller_position);
        _body->AddImpulse((realFwd * _forward * cForwardAcceleration) * cBoatMass * deltaTime, bow_position);// _body->GetCenterOfMassPosition());
        _body->AddImpulse((right * Sign(_forward) * _right * cSteerAcceleration) * cBoatMass * deltaTime, propeller_position);
        //_body->AddImpulse((forward * _forward * cForwardAcceleration + right * Sign(_forward) * _right * cSteerAcceleration) * cBoatMass * deltaTime, propeller_position);
    }
    else {
        //_body->AddImpulse((forward * _forward * cForwardAcceleration + right * Sign(_forward) * _right * cSteerAcceleration) * frictionFactor * cBoatMass * deltaTime, propeller_position);
    }

}

void Boat::OnFixedUpdate(float deltaTime)
{
}

void Boat::OnLateUpdate(float deltaTime)
{
}

void Boat::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
}

void Boat::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
}

void Boat::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
}


void Boat::OnLeftRightInput(float deltaTime, float leftRightInput)
{
    _right = -leftRightInput;
}

void Boat::OnAccelerateInput(float deltaTime, float upDownInput)
{
    _forward = upDownInput;
}

void Boat::OnBrake(float deltaTime, bool handBrakeInput)
{
    _handBrakeInput = handBrakeInput;
}

std::unordered_map<JPH::BodyID, Boat*> Boat::boatIds{};
