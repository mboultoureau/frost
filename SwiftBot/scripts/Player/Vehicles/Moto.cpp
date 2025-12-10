#include "Player/Vehicles/Moto.h"
#include "Physics/PhysicLayer.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Vehicle/MotorcycleController.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#undef min
#undef max

class MotoObjectLayerFilter : public JPH::ObjectLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inLayer) const override
    {
        return inLayer == GameLogic::ObjectLayers::NON_MOVING; // ignore camera
    }
};

class MotoBroadPhaseLayerFilter : public JPH::BroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
    {
        return inLayer == GameLogic::BroadPhaseLayers::NON_MOVING;
    }
};

using namespace JPH;
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    Moto::Moto(GameObject player) : Vehicle(player)
    {
        _vehicle = player.GetChildByName("Moto", true);
    }

    void Moto::Show()
    {
        _vehicle.SetActive(true);

        const auto& transform = _playerController.GetComponent<Transform>();
        Vec3 position = Math::vector_cast<Vec3>(transform.position);
        Quat rotation = Math::vector_cast<Quat>(transform.rotation);

        // Loosely based on:
        // https://www.whitedogbikes.com/whitedogblog/yamaha-xj-900-specs/
        const float backWheelRadius = 0.31f;
        const float backWheelWidth = 0.05f;
        const float backWheelPosZ = -0.75f;
        const float backSuspensionMinLength = 0.3f;
        const float backSuspensionMaxLength = 0.5f;
        const float backSuspensionFreq = 1.0f; // 2.0f;
        const float backBrakeTorque = 5000.0f;

        const float frontWheelRadius = 0.31f;
        const float frontWheelWidth = 0.05f;
        const float frontWheelPosZ = 0.75f;
        const float frontSuspensionMinLength = 0.3f;
        const float frontSuspensionMaxLength = 0.5f;
        const float frontSuspensionFreq = 1.0f; // 1.5f;
        const float frontBrakeTorque = 8000.0f;

        const float halfVehicleLength = 0.4f;
        const float halfVehicleWidth = 0.2f;
        const float halfVehicleHeight = 0.3f;

        const float maxSteeringAngleDegree = 30.f;

        // Angle of the front suspension
        const float casterAngleDegree = 30.f;

        RefConst<Shape> motorcycle_shape =
            OffsetCenterOfMassShapeSettings(Vec3(0, -halfVehicleHeight, 0),
                                            new BoxShape(Vec3(halfVehicleWidth, halfVehicleHeight, halfVehicleLength)))
                .Create()
                .Get();

        BodyCreationSettings motorcycleBodySettings(
            motorcycle_shape, position, rotation, EMotionType::Dynamic, ObjectLayers::PLAYER);
        motorcycleBodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
        motorcycleBodySettings.mMassPropertiesOverride.mMass = 240.0f;
        motorcycleBodySettings.mUserData = static_cast<JPH::uint64>(_playerController.GetHandle());
        motorcycleBodySettings.mRestitution = 0;
        Body* body = Physics::CreateBody(motorcycleBodySettings);
        BodyID bodyId = body->GetID();
        Physics::AddBody(bodyId, EActivation::Activate);

        // Create vehicle constraint
        VehicleConstraintSettings vehicleConstraints;
        vehicleConstraints.mDrawConstraintSize = 0.1f;
        vehicleConstraints.mMaxPitchRollAngle = DegreesToRadians(60.0f);

        // Wheels
        WheelSettingsWV* front = new WheelSettingsWV;
        front->mPosition = Vec3(0.0f, -0.9f * halfVehicleHeight, frontWheelPosZ);
        front->mMaxSteerAngle = DegreesToRadians(maxSteeringAngleDegree);
        front->mSuspensionDirection = Vec3(0, -1, Tan(DegreesToRadians(casterAngleDegree))).Normalized();
        front->mSteeringAxis = -front->mSuspensionDirection;
        front->mRadius = frontWheelRadius;
        front->mWidth = frontWheelWidth;
        front->mSuspensionMinLength = frontSuspensionMinLength;
        front->mSuspensionMaxLength = frontSuspensionMaxLength;
        front->mSuspensionSpring.mFrequency = frontSuspensionFreq;
        front->mMaxBrakeTorque = frontBrakeTorque;
        front->mLongitudinalFriction = LinearCurve();
        front->mLongitudinalFriction.AddPoint(0.0f, 0.0f);
        front->mLongitudinalFriction.AddPoint(1.0f, 0.0f);

        front->mLateralFriction = LinearCurve();
        front->mLateralFriction.AddPoint(0.0f, 1.f);
        front->mLateralFriction.AddPoint(1.0f, 1.f);

        WheelSettingsWV* back = new WheelSettingsWV;
        back->mPosition = Vec3(0.0f, -0.9f * halfVehicleHeight, backWheelPosZ);
        back->mMaxSteerAngle = 0.0f;
        back->mMaxBrakeTorque = backBrakeTorque;
        back->mSuspensionDirection = Vec3(0, -1, 0);

        back->mRadius = backWheelRadius;
        back->mWidth = backWheelWidth;
        back->mSuspensionMinLength = backSuspensionMinLength;
        back->mSuspensionMaxLength = backSuspensionMaxLength;
        back->mSuspensionSpring.mFrequency = backSuspensionFreq;

        back->mLongitudinalFriction = LinearCurve();
        back->mLongitudinalFriction.AddPoint(0.0f, 3.0f); // 3
        back->mLongitudinalFriction.AddPoint(1.0f, 3.0f);

        back->mLateralFriction = LinearCurve();
        back->mLateralFriction.AddPoint(0.0f, 2.5f); // 2.5
        back->mLateralFriction.AddPoint(1.0f, 2.5f);

        for (WheelSettingsWV* w : { front, back })
        {
            // w->mSuspensionPreloadLength = 0.0f;
            // w->mSuspensionForcePoint = w->mPosition;
            // w->mSuspensionSpring.mDamping = 0.7f;

            // w->mInertia *= 0.8f;
            // w->mAngularDamping = 0.2f;

            /*   w->mLongitudinalFriction = LinearCurve();
               w->mLongitudinalFriction.AddPoint(0.0f, 3.0f);
               w->mLongitudinalFriction.AddPoint(1.0f, 3.0f);

               w->mLateralFriction = LinearCurve();
               w->mLateralFriction.AddPoint(0.0f, 0.7f);
               w->mLateralFriction.AddPoint(1.0f, 0.7f);
               */
        }

        bool sOverrideFrontSuspensionForcePoint = true;
        if (sOverrideFrontSuspensionForcePoint)
        {
            front->mEnableSuspensionForcePoint = true;
            front->mSuspensionForcePoint = front->mPosition + front->mSuspensionDirection * front->mSuspensionMinLength;
        }
        bool sOverrideRearSuspensionForcePoint = true;
        if (sOverrideRearSuspensionForcePoint)
        {
            back->mEnableSuspensionForcePoint = true;
            back->mSuspensionForcePoint = back->mPosition + back->mSuspensionDirection * back->mSuspensionMinLength;
        }

        vehicleConstraints.mWheels = { front, back };

        // Motorcycle Controller
        MotorcycleControllerSettings* controller = new MotorcycleControllerSettings;
        controller->mEngine.mMaxTorque = 600.0f;
        controller->mEngine.mMinRPM = 1000.0f;
        controller->mEngine.mMaxRPM = 10000.0f;
        controller->mTransmission.mShiftDownRPM = 3000.0f;
        controller->mTransmission.mShiftUpRPM = 8000.0f;
        controller->mTransmission.mGearRatios = { 3.8f, 2.6f, 1.8f }; // From: https://www.blocklayer.com/rpm-gear-bikes
        controller->mTransmission.mReverseGearRatios = { -2.0f };
        controller->mTransmission.mClutchStrength = 4.0f;
        vehicleConstraints.mController = controller;

        // Differential (not really applicable to a motorcycle but we need one
        // anyway to drive it)
        controller->mDifferentials.resize(1);
        controller->mDifferentials[0].mLeftWheel = -1;
        controller->mDifferentials[0].mRightWheel = 1;
        controller->mDifferentials[0].mDifferentialRatio =
            4.5f; // Combining primary and final drive (back divided by front
                  // sprockets) from: https://www.blocklayer.com/rpm-gear-bikes

        _constraint = new VehicleConstraint(*body, vehicleConstraints);
        _constraint->SetVehicleCollisionTester(
            new VehicleCollisionTesterCastCylinder(ObjectLayers::PLAYER, 1.0f)); // Use half wheel width as convex
                                                                                 // radius so we get a rounded cylinder
        auto tester = _constraint->GetVehicleCollisionTester();
        Physics::Get().physics_system.AddConstraint(_constraint);
        Physics::Get().physics_system.AddStepListener(_constraint);
        _controller = static_cast<MotorcycleController*>(_constraint->GetController());
        _specialDriftCoolDown.Start();

        _previousLinearSpeed = body->GetLinearVelocity();
        _previousAngularSpeed = body->GetAngularVelocity();

        auto& rigidBody = _playerController.AddComponent<RigidBody>(bodyId);
        rigidBody.objectLayer = ObjectLayers::PLAYER;
        rigidBody.motionType = RigidBody::MotionType::Dynamic;
    }

    void Moto::Hide()
    {
        // Hide static mesh
        _vehicle.SetActive(false);

        // Remove physics
        if (_constraint)
        {
            Physics::Get().physics_system.RemoveConstraint(_constraint);
            Physics::Get().physics_system.RemoveStepListener(_constraint);
        }

        if (_playerController.HasComponent<RigidBody>())
        {
            _playerController.RemoveComponent<RigidBody>();
        }

        _constraint = nullptr;
        _controller = nullptr;
    }

    void Moto::OnPreFixedUpdate(float fixedDeltaTime)
    {
        if (!_controller || !_constraint)
            return;

        _ProcessBikeInput(fixedDeltaTime);

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;

        if (_right != 0.0f || _forward != 0.0f || _brake != 0.0f)
            Physics::ActivateBody(bodyId);

        _controller->SetDriverInput(_forward, _right, _brake, false);
        _controller->EnableLeanController(true);

        Vec3 bodyPosition = Vec3::sZero();
        Vec3 bodyUp = Vec3::sAxisY();
        bool bodyValid = false;

        {
            BodyLockRead lock(Physics::GetBodyLockInterface(), bodyId);
            if (lock.Succeeded())
            {
                const auto& body = lock.GetBody();
                bodyPosition = body.GetPosition();
                bodyUp = body.GetRotation() * _constraint->GetLocalUp();
                bodyValid = true;
            }
        }

        if (!bodyValid)
            return;

        Vec3 newGravityDir = Vec3::sZero();
        bool gravityOverridden = false;

        bool sOverrideGravity = true;
        if (sOverrideGravity)
        {
            SphereShape sphere(0.5f);
            sphere.SetEmbedded();

            Vec3 castDir = -3.0f * bodyUp;

            RShapeCast shape_cast(&sphere, Vec3::sOne(), RMat44::sTranslation(bodyPosition), castDir);

            ShapeCastSettings settings;
            ClosestHitCollisionCollector<CastShapeCollector> collector;

            Physics::Get().physics_system.GetNarrowPhaseQuery().CastShape(
                shape_cast, settings, bodyPosition, collector, MotoBroadPhaseLayerFilter(), MotoObjectLayerFilter());

            if (collector.HadHit())
            {
                newGravityDir = collector.mHit.mPenetrationAxis.Normalized();
                gravityOverridden = true;
            }
        }

        auto& lockInterface = Physics::GetBodyLockInterface();
        {
            BodyLockWrite lock(lockInterface, bodyId);
            if (!lock.Succeeded())
                return;

            auto& body = lock.GetBody();

            // Apply Gravity Override result
            if (gravityOverridden)
                _constraint->OverrideGravity(9.81f * newGravityDir);
            else
                _constraint->ResetGravityOverride();

            // Apply Drift Force
            bool applyDriftForceAssist = true;
            if (applyDriftForceAssist)
            {
                auto r = body.GetRotation();
                body.AddForce(r * Vec3(-_right, 0, 0) * 1000);
            }
        }

#ifdef FT_DEBUG
        // Draw wheels
        for (uint w = 0; w < 2; ++w)
        {
            const WheelSettings* settings = _constraint->GetWheels()[w]->GetSettings();
            RMat44 wheel_transform = _constraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX());
            Physics::GetDebugRenderer()->DrawCylinder(
                wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);

            Vec3 origin = wheel_transform.GetTranslation();
            Vec3 dir = _constraint->GetWheels()[w]->GetSettings()->mSuspensionDirection;
            float maxLen = _constraint->GetWheels()[w]->GetSettings()->mSuspensionMaxLength;
            Physics::GetDebugRenderer()->DrawLine(origin, origin + dir * maxLen, Color::sYellow);
        }
#endif
    }

    void Moto::OnFixedUpdate(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;
        auto& lockInterface = Physics::GetBodyLockInterface();
        {
            BodyLockRead lock(lockInterface, bodyId);
            if (!lock.Succeeded())
                return;

            auto& body = lock.GetBody();

            float linearSpeedMagnitude = body.GetLinearVelocity().Length();
            float prevlinearSpeedMagnitude = _previousLinearSpeed.Length();
            float angularSpeedMagnitude = body.GetAngularVelocity().Length();
            float prevAngularSpeedMagnitude = _previousAngularSpeed.Length();
            float speedDot = 1;

            auto& camera = _camera.GetComponent<Camera>();
            auto shakePostEffect = camera.GetEffect<ScreenShakeEffect>();
            auto radialBlurPostEffect = camera.GetEffect<RadialBlurEffect>();

            if (linearSpeedMagnitude > 0 && prevlinearSpeedMagnitude > 0)
            {
                speedDot = body.GetLinearVelocity().Normalized().Dot(_previousLinearSpeed.Normalized());
            }

            if (std::abs(linearSpeedMagnitude - prevlinearSpeedMagnitude) > _shakeLinearSpeedDiffThreshold ||
                speedDot < _shakeSpeedAngleDiffThreshold ||
                std::abs(angularSpeedMagnitude - prevAngularSpeedMagnitude) > _shakeAngularSpeedDiffThreshold)
            {
                shakePostEffect->SetEnabled(true);
                shakePostEffect->Shake(_screenShakeDuration,
                                       linearSpeedMagnitude * _screenShakeSpeedMultiplier,
                                       ScreenShakeEffect::AttenuationType::EaseOut);
            }

            _previousLinearSpeed = body.GetLinearVelocity();
            _previousAngularSpeed = body.GetAngularVelocity();

            /*
            radialBlurPostEffect->SetEnabled(true);
            radialBlurPostEffect->SetCenter(Vector2(0.5f, 0.5f));
            radialBlurPostEffect->SetStrength(linearSpeedMagnitude * _radialBlurSpeedFactor);
            */
        }
    }

    void Moto::OnBrake(bool brake)
    {
        _handBrakeInput = brake;
    }

    void Moto::OnSpecialAction(bool special)
    {
        _handBrakeInput = special;
    }

    void Moto::OnMove(float right, float forward)
    {
        _rightInput = right;
        _forwardInput = forward;
    }

    void Moto::_ProcessBikeInput(float fixedDeltaTime)
    {
        if (!_playerController.HasComponent<RigidBody>())
            return;

        auto bodyId = _playerController.GetComponent<RigidBody>().runtimeBodyID;

        // Get body
        auto& lockInterface = Physics::GetBodyLockInterface();
        {
            BodyLockRead lock(lockInterface, bodyId);
            if (!lock.Succeeded())
                return;

            const auto& body = lock.GetBody();

            _forward = 0.0f;
            _brake = 0.0f;
            if (_handBrakeInput || _specialInput)
                _brake = 1.0f;
            else if (_forwardInput > 0)
                _forward = _forwardInput;
            else if (_forwardInput < 0)
                _forward = _forwardInput;

            // Check if we're reversing direction
            if (_previousForward * _forward < 0.0f)
            {
                // Get vehicle velocity in local space to the body of the vehicle
                float velocity = (body.GetRotation().Conjugated() * body.GetLinearVelocity()).GetZ();
                if ((_forward > 0.0f && velocity < -0.1f) || (_forward < 0.0f && velocity > 0.1f))
                {
                    // Brake while we've not stopped yet
                    _forward = 0.0f;
                    _brake = 1.0f;
                }
                else
                {
                    // When we've come to a stop, accept the new direction
                    _previousForward = _forward;
                }
            }

            // Steering
            float steer_speed = 4.0f;

            float speed = body.GetLinearVelocity().Length();

            float multiplier = 1.0f;
            if (speed > 20.0f)
                multiplier = 1.0f + (speed - 20.0f) * 0.03f; // plus vite = tourne plus fort

            steer_speed *= multiplier;

            if (_rightInput > _right)
                _right = std::min(_right + steer_speed * fixedDeltaTime, _rightInput);
            else if (_rightInput < _right)
                _right = std::max(_right - steer_speed * fixedDeltaTime, _rightInput);

            if (_forwardInput == 0)
                _brake = 1.0f;

            // When leaned, we don't want to use the brakes fully as we'll spin out
            if (_brake > 0.0f)
            {
                Vec3 world_up = -Physics::Get().physics_system.GetGravity().Normalized();
                Vec3 up = body.GetRotation() * _constraint->GetLocalUp();
                Vec3 fwd = body.GetRotation() * _constraint->GetLocalForward();
                float sin_lean_angle = abs(world_up.Cross(up).Dot(fwd));
                float brake_multiplier = Square(1.0f - sin_lean_angle);
                _brake *= brake_multiplier;
            }
        }
    }

} // namespace GameLogic