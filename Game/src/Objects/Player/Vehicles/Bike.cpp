#include "Bike.h"
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
using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;


Bike::Bike(Player* player, RendererParameters params) : Vehicle( player, params)
{
	using namespace JPH;
	// Create motorcycle controller
	// Set playerbody to made body
	RenderMesh(false);
}

JPH::BodyID Bike::Appear()
{
    using namespace JPH;

    RenderMesh(true);

    // ----- Transform -----
    auto tr = _scene->GetComponent<Transform>(_player->GetPlayerID());
    Vec3 pos = Math::vector_cast<Vec3>(tr->position);
    Quat rot = Math::vector_cast<Quat>(tr->rotation);

    // Loosely based on: https://www.whitedogbikes.com/whitedogblog/yamaha-xj-900-specs/
    const float back_wheel_radius = 0.31f;
    const float back_wheel_width = 0.05f;
    const float back_wheel_pos_z = -0.75f;
    const float back_suspension_min_length = 0.3f;
    const float back_suspension_max_length = 0.5f;
    const float back_suspension_freq = 1.0f;//2.0f;
    const float back_brake_torque = 5000.0f;

    const float front_wheel_radius = 0.31f;
    const float front_wheel_width = 0.05f;
    const float front_wheel_pos_z = 0.75f;
    const float front_suspension_min_length = 0.3f;
    const float front_suspension_max_length = 0.5f;
    const float front_suspension_freq = 1.0f;// 1.5f;
    const float front_brake_torque = 8000.0f;

    const float half_vehicle_length = 0.4f;
    const float half_vehicle_width = 0.2f;
    const float half_vehicle_height = 0.3f;

    const float max_steering_angle_degree = 30.f;

    // Angle of the front suspension
    const float caster_angle_degree = 30.f;

    RefConst<Shape> motorcycle_shape = OffsetCenterOfMassShapeSettings(Vec3(0, -half_vehicle_height, 0), new BoxShape(Vec3(half_vehicle_width, half_vehicle_height, half_vehicle_length))).Create().Get();
    BodyCreationSettings motorcycle_body_settings(motorcycle_shape, pos, rot, EMotionType::Dynamic, ObjectLayers::PLAYER);
    motorcycle_body_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    motorcycle_body_settings.mMassPropertiesOverride.mMass = 240.0f;
    motorcycle_body_settings.mUserData = _player->GetPlayerID();
    motorcycle_body_settings.mRestitution = 0;
    _body = Physics::CreateBody(motorcycle_body_settings);
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

    back->mRadius = back_wheel_radius;
    back->mWidth = back_wheel_width;
    back->mSuspensionMinLength = back_suspension_min_length;
    back->mSuspensionMaxLength = back_suspension_max_length;
    back->mSuspensionSpring.mFrequency = back_suspension_freq;

    back->mLongitudinalFriction = LinearCurve();
    back->mLongitudinalFriction.AddPoint(0.0f, 3.0f); //3
    back->mLongitudinalFriction.AddPoint(1.0f, 3.0f);

    back->mLateralFriction = LinearCurve();
    back->mLateralFriction.AddPoint(0.0f, 2.5f);//2.5
    back->mLateralFriction.AddPoint(1.0f, 2.5f);

    for (WheelSettingsWV* w : { front, back })
    {
        //w->mSuspensionPreloadLength = 0.0f;
        //w->mSuspensionForcePoint = w->mPosition; 
        //w->mSuspensionSpring.mDamping = 0.7f;    

        //w->mInertia *= 0.8f;
        //w->mAngularDamping = 0.2f;

     /*   w->mLongitudinalFriction = LinearCurve();
        w->mLongitudinalFriction.AddPoint(0.0f, 3.0f);
        w->mLongitudinalFriction.AddPoint(1.0f, 3.0f);

        w->mLateralFriction = LinearCurve();
        w->mLateralFriction.AddPoint(0.0f, 0.7f);
        w->mLateralFriction.AddPoint(1.0f, 0.7f);*/
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

    // Differential (not really applicable to a motorcycle but we need one anyway to drive it)
    controller->mDifferentials.resize(1);
    controller->mDifferentials[0].mLeftWheel = -1;
    controller->mDifferentials[0].mRightWheel = 1;
    controller->mDifferentials[0].mDifferentialRatio = 4.5f; // Combining primary and final drive (back divided by front sprockets) from: https://www.blocklayer.com/rpm-gear-bikes

    mConstraint = new VehicleConstraint(*_body, vehicle);
    mConstraint->SetVehicleCollisionTester(new VehicleCollisionTesterCastCylinder(ObjectLayers::PLAYER, 1.0f)); // Use half wheel width as convex radius so we get a rounded cylinder
    auto tester = mConstraint->GetVehicleCollisionTester();
    Physics::Get().physics_system.AddConstraint(mConstraint);
    Physics::Get().physics_system.AddStepListener(mConstraint);
    mController = static_cast<MotorcycleController*>(mConstraint->GetController());
    _specialDriftCoolDown.Start();
    return _bodyId;
}



void Bike::Disappear()
{
    RenderMesh(false);
    if (mConstraint)
    {
        Physics::Get().physics_system.RemoveConstraint(mConstraint);
        Physics::Get().physics_system.RemoveStepListener(mConstraint);
    }

    Physics::RemoveAndDestroyBody(_bodyId);

    mConstraint = nullptr;
    mController = nullptr;


}

void Bike::ProcessBikeInput(float deltaTime) {
    using namespace JPH;
    // Determine acceleration and brake
    _forward = 0.0f;
    _brake = 0.0f;
    if (_handBrakeInput || _specialInput)
        _brake = 1.0f;
    else if (_upDownInput > 0)
        _forward = _upDownInput;
    else if (_upDownInput < 0)
        _forward = _upDownInput;

    // Check if we're reversing direction
    if (_previousForward * _forward < 0.0f)
    {
        // Get vehicle velocity in local space to the body of the vehicle
        float velocity = (_body->GetRotation().Conjugated() * _body->GetLinearVelocity()).GetZ();
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

    float speed = _body->GetLinearVelocity().Length();

    float multiplier = 1.0f;
    if (speed > 20.0f)
        multiplier = 1.0f + (speed - 20.0f) * 0.03f;  // plus vite = tourne plus fort

    steer_speed *= multiplier;

    if (_leftRightInput > _right)
        _right = std::min(_right + steer_speed * deltaTime, _leftRightInput);
    else if (_leftRightInput < _right)
        _right = std::max(_right - steer_speed * deltaTime, _leftRightInput);

    // When leaned, we don't want to use the brakes fully as we'll spin out
    if (_brake > 0.0f)
    {
        Vec3 world_up = -Physics::Get().physics_system.GetGravity().Normalized();
        Vec3 up = _body->GetRotation() * mConstraint->GetLocalUp();
        Vec3 fwd = _body->GetRotation() * mConstraint->GetLocalForward();
        float sin_lean_angle = abs(world_up.Cross(up).Dot(fwd));
        float brake_multiplier = Square(1.0f - sin_lean_angle);
        _brake *= brake_multiplier;
    }
}



void Bike::OnPreFixedUpdate(float deltaTime)
{
    class SpecifiedObjectLayerFilter : public JPH::ObjectLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer) const override
        {
            return inLayer == ObjectLayers::NON_MOVING;   // ignore la camera
        }
    };

    class SpecifiedBroadPhaseLayerFilter : public JPH::BroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const override
        {
            return inLayer == Frost::BroadPhaseLayers::NON_MOVING;
        }
    };

    using namespace JPH;
    if (!mController || !mConstraint)
        return;
 
    ProcessBikeInput(deltaTime);

   
    // On user input, assure that the motorcycle is active
    if (_right != 0.0f || _forward != 0.0f || _brake != 0.0f)
        Physics::ActivateBody(_bodyId);

    // Pass the input on to the constraint
    mController->SetDriverInput(_forward, _right, _brake, false);
    mController->EnableLeanController(true);

    bool sOverrideGravity = true;
    if (sOverrideGravity)
    {
        // When overriding gravity is requested, we cast a sphere downwards (opposite to the previous up position) and use the contact normal as the new gravity direction
        SphereShape sphere(0.5f);
        sphere.SetEmbedded();
        RShapeCast shape_cast(&sphere, Vec3::sOne(), RMat44::sTranslation(_body->GetPosition()), -3.0f * mConstraint->GetWorldUp());
        ShapeCastSettings settings;
        ClosestHitCollisionCollector<CastShapeCollector> collector;
        Physics::Get().physics_system.GetNarrowPhaseQuery().CastShape(shape_cast, settings, _body->GetPosition(), collector, SpecifiedBroadPhaseLayerFilter(), SpecifiedObjectLayerFilter());
        if (collector.HadHit())
            mConstraint->OverrideGravity(9.81f * collector.mHit.mPenetrationAxis.Normalized());
        else
            mConstraint->ResetGravityOverride();
    }

    bool applyDriftForceAssist = true;
    if (applyDriftForceAssist)
    {
        auto r = _body->GetRotation();
       _body->AddForce(r*Vec3(-_right,0,0) * 1000);

    }

#ifdef FT_DEBUG
    // Draw our wheels (this needs to be done in the pre update since we draw the bodies too in the state before the step)
    for (uint w = 0; w < 2; ++w)
    {
        const WheelSettings* settings = mConstraint->GetWheels()[w]->GetSettings();
        RMat44 wheel_transform = mConstraint->GetWheelWorldTransform(w, Vec3::sAxisY(), Vec3::sAxisX()); // The cylinder we draw is aligned with Y so we specify that as rotational axis
        Physics::GetDebugRenderer()->DrawCylinder(wheel_transform, 0.5f * settings->mWidth, settings->mRadius, Color::sGreen);

        RMat44 tf = mConstraint->GetWheelWorldTransform(w, JPH::Vec3::sAxisY(), JPH::Vec3::sAxisX());
        Vec3 origin = tf.GetTranslation();
        Vec3 dir = mConstraint->GetWheels()[w]->GetSettings()->mSuspensionDirection;
        float maxLen = mConstraint->GetWheels()[w]->GetSettings()->mSuspensionMaxLength;
        Physics::GetDebugRenderer()->DrawLine(origin, origin + dir * maxLen, Color::sYellow);
    }
#endif
}

void Bike::OnFixedUpdate(float deltaTime)
{
}

void Bike::OnLateUpdate(float deltaTime)
{
    auto vel = _body->GetLinearVelocity();
    if (vel.Length() > _maxSpeed && _specialDriftCoolDown.GetDurationAs<std::chrono::milliseconds>() == 0ms)
        _body->SetLinearVelocity(vel * (1-deltaTime));
    
    if (_specialDriftCoolDown.GetDurationAs<std::chrono::milliseconds>() == _driftCoolDownDuration) {
        _specialDriftCoolDown.Pause();
    }
}

void Bike::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
}

void Bike::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
}

void Bike::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
}


void Bike::OnLeftRightInput(float deltaTime, float leftRightInput)
{
    _leftRightInput = -leftRightInput;
}

void Bike::OnAccelerateInput(float deltaTime, float upDownInput)
{
    _upDownInput = upDownInput;
}

void Bike::OnBrake(float deltaTime, bool handBrakeInput)
{
    _handBrakeInput = handBrakeInput;
}

void Bike::GiveBoost() {
    using namespace JPH;

    auto time = std::min(static_cast<float>(_specialDriftTimer.GetDurationAs<std::chrono::milliseconds>().count()), _specialDriftMaxDuration);
    _specialDriftTimer.Pause();
    auto dir = Physics::GetBodyInterface().GetRotation(_bodyId) * Vec3(0, 0, 1);
    float antiMaxSpeedFactor = (_maxSpeed - _body->GetLinearVelocity().Length()) / _maxSpeed + 0.1f;

    Physics::GetBodyInterface().AddForce(_bodyId, dir * time * _speedAtDriftStart * _specialDriftPower * antiMaxSpeedFactor);
    // Physics::GetBodyInterface().AddTorque(_bodyId, Vec3{ 0, _leftRightInput * _specialDriftRotationPower, 0 });
    _specialDriftCoolDown.Start();
};

void Bike::OnSpecial(float deltaTime, bool specialInput)
{
    using namespace JPH;


    bool canDrift = (_specialDriftCoolDown.GetDurationAs<std::chrono::milliseconds>() >= _driftCoolDownDuration);
    // Drift
    if (_specialInput != specialInput && canDrift ) {
        // On key pressed : start drift
        if (specialInput)
        {
            if (_leftRightInput != 0) {
                _specialDriftTimer.Start();
                _speedAtDriftStart = Physics::GetBodyInterface().GetLinearVelocity(_bodyId).Length();
            }
        }
        // On Key released : end drift
        else
        {
            GiveBoost();
        }
    }
    //else if (!_specialInput && specialInput && canDrift) GiveBoost();

    _specialInput = specialInput;
}
