#include "Player/Vehicles/Moto.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>

using namespace JPH;
using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    Moto::Moto(GameObject player, GameObject vehicle) : Vehicle(player, vehicle)
    {
        /*
        const auto& transform = _player.GetComponent<Transform>();
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
        motorcycleBodySettings.mUserData = _player.GetHandle();
        motorcycleBodySettings.mRestitution = 0;
        _body = Physics::CreateBody(motorcycleBodySettings);
        _bodyId = _body->GetID();
        Physics::AddBody(_bodyId, EActivation::Activate);

        // Create vehicle constraint
        VehicleConstraintSettings vehicle;
        vehicle.mDrawConstraintSize = 0.1f;
        vehicle.mMaxPitchRollAngle = DegreesToRadians(60.0f);

        // Wheels
        WheelSettingsWV* front = new WheelSettingsWV;
        front->mPosition = Vec3(0.0f, -0.9f * half_vehicle_height, front_wheel_pos_z);
        front->mMaxSteerAngle = DegreesToRadians(max_steering_angle_degree);
        front->mSuspensionDirection = Vec3(0, -1, Tan(DegreesToRadians(caster_angle_degree))).Normalized();
        front->mSteeringAxis = -front->mSuspensionDirection;
        front->mRadius = front_wheel_radius;
        front->mWidth = front_wheel_width;
        front->mSuspensionMinLength = front_suspension_min_length;
        front->mSuspensionMaxLength = front_suspension_max_length;
        front->mSuspensionSpring.mFrequency = front_suspension_freq;
        front->mMaxBrakeTorque = front_brake_torque;
        front->mLongitudinalFriction = LinearCurve();
        front->mLongitudinalFriction.AddPoint(0.0f, 0.0f);
        front->mLongitudinalFriction.AddPoint(1.0f, 0.0f);

        front->mLateralFriction = LinearCurve();
        front->mLateralFriction.AddPoint(0.0f, 1.f);
        front->mLateralFriction.AddPoint(1.0f, 1.f);

        WheelSettingsWV* back = new WheelSettingsWV;
        back->mPosition = Vec3(0.0f, -0.9f * half_vehicle_height, back_wheel_pos_z);
        back->mMaxSteerAngle = 0.0f;
        back->mMaxBrakeTorque = back_brake_torque;
        back->mSuspensionDirection = Vec3(0, -1, 0);

        back->mRadius = backWheelRadius;
        back->mWidth = backWheelWidth;
        back->mSuspensionMinLength = back_suspension_min_length;
        back->mSuspensionMaxLength = back_suspension_max_length;
        back->mSuspensionSpring.mFrequency = back_suspension_freq;

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

        vehicle.mWheels = { front, back };

        // ----- Motorcycle Controller -----
        MotorcycleControllerSettings* controller = new MotorcycleControllerSettings;
        controller->mEngine.mMaxTorque = 600.0f;
        controller->mEngine.mMinRPM = 1000.0f;
        controller->mEngine.mMaxRPM = 10000.0f;
        controller->mTransmission.mShiftDownRPM = 3000.0f;
        controller->mTransmission.mShiftUpRPM = 8000.0f;
        controller->mTransmission.mGearRatios = { 3.8f, 2.6f, 1.8f }; // From: https://www.blocklayer.com/rpm-gear-bikes
        controller->mTransmission.mReverseGearRatios = { -2.0f };
        controller->mTransmission.mClutchStrength = 4.0f;
        vehicle.mController = controller;

        // Differential (not really applicable to a motorcycle but we need one
        // anyway to drive it)
        controller->mDifferentials.resize(1);
        controller->mDifferentials[0].mLeftWheel = -1;
        controller->mDifferentials[0].mRightWheel = 1;
        controller->mDifferentials[0].mDifferentialRatio =
            4.5f; // Combining primary and final drive (back divided by front
                  // sprockets) from: https://www.blocklayer.com/rpm-gear-bikes

        mConstraint = new VehicleConstraint(*_body, vehicle);
        mConstraint->SetVehicleCollisionTester(
            new VehicleCollisionTesterCastCylinder(ObjectLayers::PLAYER, 1.0f)); // Use half wheel width as convex
                                                                                 // radius so we get a rounded cylinder
        auto tester = mConstraint->GetVehicleCollisionTester();
        Physics::Get().physics_system.AddConstraint(mConstraint);
        Physics::Get().physics_system.AddStepListener(mConstraint);
        mController = static_cast<MotorcycleController*>(mConstraint->GetController());
        _specialDriftCoolDown.Start();

        _previousLinearSpeed = _body->GetLinearVelocity();
        _previousAngularSpeed = _body->GetAngularVelocity();
        return _bodyId;
        */
    }

    void Moto::Show() {}

    void Moto::Hide() {}
} // namespace GameLogic