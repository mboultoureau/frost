#include "Boat.h"
#include "Physics/PhysicLayer.h"
#include "GameState/GameState.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Frost/Scene/Components/RigidBody.h>
#include <Frost/Scene/Components/WorldTransform.h>

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    Boat::Boat(GameObject player) : Vehicle(player)
    {
        _vehicle = player.GetChildByName("Boat", true);
    }

    void Boat::Show()
    {
        _vehicle.SetActive(true);

        const auto& transform = _playerController.GetComponent<Transform>();
        Vec3 pos = Math::vector_cast<Vec3>(transform.position);
        Quat rot = Math::vector_cast<Quat>(transform.rotation);

        // Points
        std::vector<Vec3> hullPoints = { Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
                                         Vec3(cHalfBoatTopWidth, cHalfBoatHeight, -cHalfBoatLength),
                                         Vec3(-cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
                                         Vec3(cHalfBoatTopWidth, cHalfBoatHeight, cHalfBoatLength),
                                         Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
                                         Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, -cHalfBoatLength),
                                         Vec3(-cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
                                         Vec3(cHalfBoatBottomWidth, -cHalfBoatHeight, cHalfBoatLength),
                                         Vec3(0, cHalfBoatHeight, cHalfBoatLength + cBoatBowLength) };

        // Fix: Use data() and size() for Jolt array constructor
        ConvexHullShapeSettings boatHullSettings(hullPoints.data(), static_cast<int>(hullPoints.size()));
        boatHullSettings.SetEmbedded();

        OffsetCenterOfMassShapeSettings comOffset(Vec3(0, -cHalfBoatHeight, 0), &boatHullSettings);
        comOffset.SetEmbedded();

        BodyCreationSettings boatSettings(&comOffset, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
        boatSettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
        boatSettings.mMassPropertiesOverride.mMass = cBoatMass;
        boatSettings.mAngularDamping = 0.8f;
        boatSettings.mLinearDamping = 0.3f;
        boatSettings.mFriction = cBoatFriction;
        boatSettings.mUserData = static_cast<JPH::uint64>(_playerController.GetHandle());

        Body* body = Physics::CreateBody(boatSettings);
        BodyID bodyId = body->GetID();
        Physics::AddBody(bodyId, EActivation::Activate);

        auto& rigidBody = _playerController.AddComponent<RigidBody>(bodyId);
        rigidBody.objectLayer = ObjectLayers::PLAYER;
        rigidBody.motionType = RigidBody::MotionType::Dynamic;

        _isBodyValid = true;
    }

    void Boat::Hide()
    {
        _vehicle.SetActive(false);

        if (_playerController.HasComponent<RigidBody>())
        {
            _playerController.RemoveComponent<RigidBody>();
        }
        _isBodyValid = false;
    }

    void Boat::OnMove(float right, float forward)
    {
        _rightInput = -right;
        _forwardInput = forward > 0 ? forward : 0;
    }

    void Boat::OnBrake(bool brake)
    {
        _handBrakeInput = brake;
    }

    void Boat::OnSpecialAction(bool special) {}

    void Boat::OnPreFixedUpdate(float fixedDeltaTime)
    {
        _ProcessPhysics(fixedDeltaTime);
    }

    void Boat::_ProcessPhysics(float fixedDeltaTime)
    {
        if (!_isBodyValid || !_playerController.HasComponent<RigidBody>())
            return;

        const auto& playerData = GameState::Get().GetPlayerData(_player);
        if (!playerData.isInWater)
            return;

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;

        _currentForward = _forwardInput;
        _currentRight = _rightInput;

        if (_currentRight != 0.0f || _currentForward != 0.0f)
            Physics::ActivateBody(bodyId);

        auto& lockInterface = Physics::GetBodyLockInterface();
        BodyLockWrite lock(lockInterface, bodyId);
        if (!lock.Succeeded())
            return;

        auto& body = lock.GetBody();

        Vec3 pos = body.GetPosition();
        Quat rot = body.GetRotation();

        RVec3 propellerPosition = pos + rot * Vec3(0, -cHalfBoatHeight, -cHalfBoatLength);
        RVec3 bowPosition = pos + rot * Vec3(0, -cHalfBoatHeight, cHalfBoatLength);

        Vec3 forwardDir = rot.RotateAxisZ();
        Vec3 realFwd = Vec3(forwardDir.GetX(), 0, forwardDir.GetZ()).NormalizedOr(Vec3::sAxisZ());
        Vec3 rightDir = rot.RotateAxisX();

        body.AddImpulse((realFwd * _currentForward * cForwardAcceleration) * cBoatMass * fixedDeltaTime, bowPosition);

        body.AddImpulse((rightDir * Sign(_currentForward) * _currentRight * cSteerAcceleration) * cBoatMass *
                            fixedDeltaTime,
                        propellerPosition);

        if (_handBrakeInput)
        {
            Vec3 currentVelocity = body.GetLinearVelocity();
            Vec3 horizontalVelocity = { currentVelocity.GetX(), 0, currentVelocity.GetZ() };
            Vec3 brakeDiff = horizontalVelocity * cBoatBrakeStrength * fixedDeltaTime;
            body.SetLinearVelocity(currentVelocity - brakeDiff);
        }
    }

    void Boat::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& lockInterface = Physics::GetBodyLockInterface();
        BodyLockRead lock(lockInterface, bodyId);
        if (lock.Succeeded())
        {
            const auto& body = lock.GetBody();
            float speed = body.GetLinearVelocity().Length();

            auto& camera = _camera.GetComponent<Camera>();
            if (auto radialBlur = camera.GetEffect<RadialBlurEffect>())
            {
                radialBlur->SetStrength(speed * cRadialBlurSpeedFactor);
            }
        }
    }
} // namespace GameLogic