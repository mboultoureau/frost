#include "Boat.h"
#include "../../../Game.h"
#include "../../../MainLayer.h"
#include "../PlayerCamera.h"
#include "../../../Physics/PhysicsLayer.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/WorldTransform.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>

// TODO: remove windows header from polluting all the code
#undef max
#undef min
#include "../../Water.h"
#include <DirectXMath.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <algorithm>
#include <cmath>
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

JPH::BodyID
Boat::Appear()
{
    using namespace JPH;

    RenderMesh(true);
    _player->SetIsInWater(false);

    // ----- Transform -----
    auto tr = _scene->GetComponent<Transform>(_player->GetPlayerID());
    Vec3 pos = Math::vector_cast<Vec3>(tr->position);
    Quat rot = Math::vector_cast<Quat>(tr->rotation);

    // Create boat
    ConvexHullShapeSettings boat_hull;
    boat_hull.mPoints = { Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
                          Vec3(cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
                          Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
                          Vec3(cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
                          Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
                          Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
                          Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
                          Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
                          Vec3(0, cHalfBoatHeight, cHalfBoatLength + cBoatBowLength) };
    boat_hull.SetEmbedded();
    OffsetCenterOfMassShapeSettings com_offset(Vec3(0, -cHalfBoatHeight, 0), &boat_hull);
    com_offset.SetEmbedded();
    BodyCreationSettings boat(&com_offset, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
    boat.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    boat.mMassPropertiesOverride.mMass = cBoatMass;
    boat.mAngularDamping = 0.1f;
    // boat.mGravityFactor = 15.f;
    boat.mFriction = cBoatFriction;
    _bodyId = Physics::CreateAndAddBody(boat, _player->GetPlayerID(), EActivation::Activate);

    Boat::RegisterBoat(_bodyId, this);
    _isBodyValid = true;
    return _bodyId;
}

void
Boat::Disappear()
{
    if (!_player->GetPlayerID().IsValid())
        return;

    RenderMesh(false);

    if (_player->GetPlayerID().HasComponent<RigidBody>())
        _player->GetPlayerID().RemoveComponent<RigidBody>();

    Boat::UnregisterBoat(_bodyId);
    _isBodyValid = false;
}

void
Boat::ProcessBoatInput(float deltaTime)
{
}

void
Boat::OnPreFixedUpdate(float deltaTime)
{
}

void
Boat::OnFixedUpdate(float deltaTime)
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    Player* player = mainLayer->GetPlayer();

    auto playerCamera = player->GetCamera();

    playerCamera->SetRadialBlurStrength(Physics::GetBodyInterface().GetLinearVelocity(_bodyId).Length() *
                                        cRadialBlurSpeedFactor);
}

void
Boat::OnLateUpdate(float deltaTime)
{
}

void
Boat::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    if (params.inBody1.IsSensor() || params.inBody2.IsSensor())
        return;

    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    Player* player = mainLayer->GetPlayer();

    auto playerCamera = player->GetCamera();

    playerCamera->Shake(cScreenShakeDuration,
                        Physics::GetBodyInterface()
                                .GetLinearVelocity(_player->GetPlayerID().GetComponent<RigidBody>().runtimeBodyID)
                                .Length() *
                            cScreenShakeSpeedMultiplier,
                        ScreenShakeEffect::AttenuationType::EaseOut);
}

void
Boat::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
    using namespace JPH;
    if (!_isBodyValid || !_currentWater || !_player->IsInWater())
        return;
    ProcessBoatInput(deltaTime);

    // On user input, assure that the boat is active
    if (_right != 0.0f || _forward != 0.0f)
        Physics::ActivateBody(_bodyId);

    auto wTransform = _player->GetPlayerID().GetComponent<WorldTransform>();
    Quat rot = vector_cast<Quat>(wTransform.rotation);
    Vec3 pos = vector_cast<Vec3>(wTransform.position);

    RVec3 propeller_position = pos + rot * Vec3(0, -cHalfBoatHeight, -cHalfBoatLength);
    RVec3 bow_position = pos + rot * Vec3(0, -cHalfBoatHeight, cHalfBoatLength);

    auto forward = rot.RotateAxisZ();
    auto realFwd = Vec3(forward.GetX(), 0, forward.GetZ());
    Vec3 right = rot.RotateAxisX();

    Physics::GetBodyInterface().AddImpulse(_bodyId,
                                           (realFwd * _forward * cForwardAcceleration) * cBoatMass * deltaTime,
                                           bow_position); // _body->GetCenterOfMassPosition());
    Physics::GetBodyInterface().AddImpulse(
        _bodyId, (right * Sign(_forward) * _right * cSteerAcceleration) * cBoatMass * deltaTime, propeller_position);

    if (_handBrakeInput != 0)
    {
        auto currentVelocity = Physics::GetBodyInterface().GetLinearVelocity(_bodyId);

        JPH::Vec3 horizontalVelocity = { currentVelocity.GetX(), 0, currentVelocity.GetZ() };

        JPH::Vec3 brakeDiff = horizontalVelocity * cBoatBrakeStrength * deltaTime;

        Physics::GetBodyInterface().SetLinearVelocity(_bodyId, currentVelocity - brakeDiff);
    }
}

void
Boat::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
}

void
Boat::OnLeftRightInput(float deltaTime, float leftRightInput)
{
    _right = -leftRightInput;
}

void
Boat::OnAccelerateInput(float deltaTime, float upDownInput)
{
    _forward = upDownInput > 0 ? upDownInput : 0;
}

void
Boat::OnBrake(float deltaTime, bool handBrakeInput)
{
    _handBrakeInput = handBrakeInput;
}

std::unordered_map<JPH::BodyID, Boat*> Boat::boatIds{};
