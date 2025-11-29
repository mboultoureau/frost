#include "Plane.h"
#include "../../../Game.h"
#include "../../../Physics/PhysicsLayer.h"

#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Scene/Components/WorldTransform.h"

#include <Jolt/Math/Math.h>
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
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;

Plane::Plane(Player* player, RendererParameters params) : Vehicle(player, params), currentSpeed(baseForwardSpeed)
{
    RenderMesh(false);
}

void
Plane::OnBrake(float deltaTime, bool handBrakeInput)
{
}

void
Plane::OnLeftRightInput(float deltaTime, float leftRightInput)
{
    this->leftRightInput = leftRightInput;
}

void
Plane::OnAccelerateInput(float deltaTime, float upDownInput)
{
    this->upDownInput = -upDownInput;
}

void
Plane::OnPreFixedUpdate(float deltaTime)
{
    using namespace JPH;

    if (_player->IsInWater())
    {
        return;
    }

    if (inContinuousCollision)
    {
        return;
    }

    auto& bodyInterface = Physics::GetBodyInterface();

    UpdateInternalStateFromBody();

    currentSpeed = bodyInterface.GetLinearVelocity(_bodyId).Length();

    // Angles cibles selon les inputs
    float targetPitch = -upDownInput * maxPitchAngle;
    float targetRoll = -leftRightInput * maxRollAngle;

    // Interpolation progressive vers les angles cibles
    currentPitch = MoveTowards(currentPitch, targetPitch, pitchSpeed * deltaTime);
    currentRoll = MoveTowards(currentRoll, targetRoll, rollSpeed * deltaTime);

    // Le yaw change proportionnellement au roll actuel
    currentYaw -= currentRoll * yawFromRoll * deltaTime;

    // Vitesse selon le pitch
    if (upDownInput < 0)
    {
        float targetSpeed = maxForwardSpeed;
        float speedLerp = 1.0f - std::exp(-speedSmoothing * deltaTime);
        currentSpeed += (targetSpeed - currentSpeed) * speedLerp;
    }
    else if (upDownInput > 0)
    {
        float targetSpeed = minForwardSpeed;
        float speedLerp = 1.0f - std::exp(-speedSmoothing * deltaTime);
        currentSpeed += (targetSpeed - currentSpeed) * speedLerp;
    }
    else
    {
        float targetSpeed = baseForwardSpeed;
        if (currentSpeed > targetSpeed)
        {
            currentSpeed = std::max(targetSpeed, currentSpeed - noInputDeceleration * deltaTime);
        }
        else if (currentSpeed < targetSpeed)
        {
            currentSpeed = std::min(targetSpeed, currentSpeed + noInputDeceleration * deltaTime);
        }
    }
    currentSpeed = std::clamp(currentSpeed, minForwardSpeed, maxForwardSpeed);

    // Taux vertical
    float verticalRate = -baseSinkRate;
    if (upDownInput < 0)
    {
        verticalRate = -diveSinkRate;
    }
    else if (upDownInput > 0)
    {
        verticalRate = climbLiftRate;
    }

    JPH::Quat yawQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), currentYaw);
    JPH::Quat pitchQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisX(), currentPitch);
    JPH::Quat rollQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), currentRoll);
    JPH::Quat newRotation = yawQuat * pitchQuat * rollQuat;

    JPH::Vec3 horizontalDir(std::sin(currentYaw), 0, std::cos(currentYaw));
    JPH::Vec3 finalVelocity = horizontalDir * currentSpeed + JPH::Vec3(0, verticalRate, 0);

    bodyInterface.SetRotation(_bodyId, newRotation, JPH::EActivation::Activate);
    bodyInterface.SetLinearVelocity(_bodyId, finalVelocity);
}

void
Plane::OnLateUpdate(float deltaTime)
{
    if (collisionCoolDownTimer.GetDurationAs<std::chrono::milliseconds>() > collisionCoolDown)
    {
        collisionCoolDownTimer.Start();
        collisionCoolDownTimer.Pause();
        inContinuousCollision = false;
    }
    // FT_INFO(collisionCoolDownTimer.GetDuration().);
}

void
Plane::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    inContinuousCollision = true;
    collisionCoolDownTimer.Start();
}

void
Plane::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
    // inContinuousCollision = false;
    // FT_INFO("collision exit");
}

JPH::BodyID
Plane::Appear()
{
    using namespace JPH;
    RenderMesh(true);
    _player->SetIsInWater(false);
    auto wTransform = _scene->GetComponent<Transform>(_player->GetPlayerID());
    auto joltPos = Math::vector_cast<Vec3>(wTransform->position);
    auto joltRot = Math::vector_cast<Quat>(wTransform->rotation);

    currentPitch = joltRot.GetRotationAngle(Vec3(1, 0, 0));
    currentYaw = joltRot.GetRotationAngle(Vec3(0, 1, 0));
    // currentRoll = joltRot.GetRotationAngle(Vec3(0, 0, 1));

    // JPH::ShapeRefC shape = new JPH::BoxShape(RVec3(1, 1, 1));
    JPH::ShapeRefC shape = new JPH::SphereShape(1.0f);

    BodyCreationSettings bodySettings(shape, joltPos, joltRot, EMotionType::Dynamic, ObjectLayers::PLAYER);
    bodySettings.mRestitution = .5f;
    bodySettings.mAllowSleeping = false;
    bodySettings.mFriction = 130.f;
    bodySettings.mGravityFactor = 0.2f;
    bodySettings.mAllowedDOFs = EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
    _bodyId = Physics::CreateAndAddBody(bodySettings, _player->GetPlayerID(), EActivation::Activate);

    inContinuousCollision = false;
    return _bodyId;
}

void
Plane::Disappear()
{
    using namespace JPH;
    RenderMesh(false);
    Physics::RemoveAndDestroyBody(_bodyId);
}
