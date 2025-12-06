#include "Plane.h"
#include "../../../Game.h"
#include "../../../MainLayer.h"
#include "../PlayerCamera.h"
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

Plane::Plane(Player* player, RendererParameters params) : Vehicle(player, params), _currentSpeed(_baseForwardSpeed)
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
    this->_leftRightInput = leftRightInput;
}

void
Plane::OnAccelerateInput(float deltaTime, float upDownInput)
{
    this->_upDownInput = -upDownInput;
}

void
Plane::OnPreFixedUpdate(float deltaTime)
{
    using namespace JPH;

    if (_player->IsInWater())
    {
        return;
    }

    if (_inContinuousCollision)
    {
        return;
    }

    auto& bodyInterface = Physics::GetBodyInterface();

    UpdateInternalStateFromBody(_justAppeared);

    JPH::Vec3 horizontalVelocity = { bodyInterface.GetLinearVelocity(_bodyId).GetX(),
                                     0,
                                     bodyInterface.GetLinearVelocity(_bodyId).GetZ() };

    _currentSpeed = horizontalVelocity.Length();

    // Angles cibles selon les inputs
    float targetPitch = -_upDownInput * _maxPitchAngle;
    float targetRoll = -_leftRightInput * _maxRollAngle;

    // Interpolation progressive vers les angles cibles
    _currentPitch = MoveTowards(_currentPitch, targetPitch, _pitchSpeed * deltaTime);
    _currentRoll = MoveTowards(_currentRoll, targetRoll, _rollSpeed * deltaTime);

    // Le yaw change proportionnellement au roll actuel
    _currentYaw -= _currentRoll * _yawFromRoll * deltaTime;

    // Vitesse selon le pitch
    if (_upDownInput < 0)
    {
        float targetSpeed = _maxForwardSpeed;
        float speedLerp = 1.0f - std::exp(-_downwardSpeedSmoothing * deltaTime);
        _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
    }
    else if (_upDownInput > 0)
    {
        float targetSpeed = _minForwardSpeed;
        float speedLerp = 1.0f - std::exp(-_upwardSpeedSmoothing * deltaTime);
        _currentSpeed += (targetSpeed - _currentSpeed) * speedLerp;
    }
    else
    {
        float targetSpeed = _baseForwardSpeed;
        if (_currentSpeed > targetSpeed)
        {
            _currentSpeed = std::max(targetSpeed, _currentSpeed - _noInputDeceleration * deltaTime);
        }
        else if (_currentSpeed < targetSpeed)
        {
            _currentSpeed = std::min(targetSpeed, _currentSpeed + _noInputDeceleration * deltaTime);
        }
    }
    _currentSpeed = std::clamp(_currentSpeed, _minForwardSpeed, _maxForwardSpeed);

    // Taux vertical
    float verticalRate = -_baseSinkRate;
    if (_upDownInput < 0)
    {
        verticalRate = -_diveSinkRate;
    }
    else if (_upDownInput > 0)
    {
        verticalRate = _currentSpeed - _climbLiftRateOffset;
    }

    JPH::Quat yawQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), _currentYaw);
    JPH::Quat pitchQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisX(), _currentPitch);
    JPH::Quat rollQuat = JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), _currentRoll);
    JPH::Quat newRotation = yawQuat * pitchQuat * rollQuat;

    JPH::Vec3 horizontalDir(std::sin(_currentYaw), 0, std::cos(_currentYaw));
    JPH::Vec3 finalVelocity = horizontalDir * _currentSpeed + JPH::Vec3(0, verticalRate, 0);

    bodyInterface.SetRotation(_bodyId, newRotation, JPH::EActivation::Activate);
    bodyInterface.SetLinearVelocity(_bodyId, finalVelocity);

    _justAppeared = false;
}

void
Plane::OnFixedUpdate(float fixedDeltaTime)
{
    auto& scene = Game::GetScene();
    auto mainLayer = Game::GetMainLayer();
    Player* player = mainLayer->GetPlayer();

    auto playerCamera = player->GetCamera();

    playerCamera->SetRadialBlurStrength(_currentSpeed * _radialBlurSpeedFactor);
}

void
Plane::OnLateUpdate(float deltaTime)
{
    if (_collisionCoolDownTimer.GetDurationAs<std::chrono::milliseconds>() > _collisionCoolDown)
    {
        _collisionCoolDownTimer.Start();
        _collisionCoolDownTimer.Pause();
        _inContinuousCollision = false;
    }
    // FT_INFO(collisionCoolDownTimer.GetDuration().);
}

void
Plane::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    if (!_inContinuousCollision)
    {
        auto& scene = Game::GetScene();
        auto mainLayer = Game::GetMainLayer();
        Player* player = mainLayer->GetPlayer();

        auto playerCamera = player->GetCamera();

        playerCamera->Shake(_screenShakeDuration,
                            _currentSpeed * _screenShakeSpeedMultiplier,
                            ScreenShakeEffect::AttenuationType::EaseOut);
    }

    if (params.inBody1.IsSensor() || params.inBody2.IsSensor())
        return;

    _inContinuousCollision = true;
    _collisionCoolDownTimer.Start();
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

    JPH::ShapeRefC shape = new JPH::SphereShape(1.0f);

    BodyCreationSettings bodySettings(shape, joltPos, joltRot, EMotionType::Dynamic, ObjectLayers::PLAYER);
    bodySettings.mRestitution = .5f;
    bodySettings.mAllowSleeping = false;
    bodySettings.mFriction = 130.f;
    bodySettings.mGravityFactor = 0.2f;
    bodySettings.mAllowedDOFs = EAllowedDOFs::TranslationX | EAllowedDOFs::TranslationY | EAllowedDOFs::TranslationZ;
    _bodyId = Physics::CreateAndAddBody(bodySettings, _player->GetPlayerID(), EActivation::Activate);

    _inContinuousCollision = false;
    _justAppeared = true;
    return _bodyId;
}

void
Plane::Disappear()
{
    if (!_player->GetPlayerID().IsValid())
        return;

    using namespace JPH;
    RenderMesh(false);

    if (_player->GetPlayerID().HasComponent<RigidBody>())
        _player->GetPlayerID().RemoveComponent<RigidBody>();
}
