#include "Water.h"

#include "Frost.h"

#include "Player/Vehicles/Boat.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "../Game.h"
#include "../Physics/PhysicsLayer.h"
using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void WaterScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
	auto id1 = params.inBody1.GetID();
	auto id2 = params.inBody2.GetID();
	if (Boat::IsBoatID(id1)) Boat::SetBoatWater(id1, _water);
	if (Boat::IsBoatID(id2)) Boat::SetBoatWater(id2, _water);
}

void WaterScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
	using namespace JPH;

	auto id = (_water->GetBodyId() == params.inBody1.GetID()) ? params.inBody2.GetID() : params.inBody1.GetID();

	BodyLockWrite body_lock(Physics::Get().physics_system.GetBodyLockInterface(), id);
	Body& body = body_lock.GetBody();
	if (body.IsActive())
	{
		// Use center of mass position to determine water surface position (you could test multiple points on the actual shape of the boat to get a more accurate result)
		RVec3 surface_position = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition());

		// Crude way of approximating the surface normal
		RVec3 p2 = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(0, 0, 1));
		RVec3 p3 = _water->GetWaterSurfacePosition(body.GetCenterOfMassPosition() + Vec3(1, 0, 0));
		Vec3 surface_normal = Vec3(p2 - surface_position).Cross(Vec3(p3 - surface_position)).Normalized();

		// Determine buoyancy and drag
		float buoyancy, linear_drag, angular_drag;
		if (Boat::IsBoatID(id))
		{
			buoyancy = cBoatBuoyancy;
			linear_drag = cBoatLinearDrag;
			angular_drag = cBoatAngularDrag;
		}
		else
		{
			buoyancy = cBarrelBuoyancy;
			linear_drag = cBarrelLinearDrag;
			angular_drag = cBarrelAngularDrag;
		}

		// Apply buoyancy to the body
		body.ApplyBuoyancyImpulse(surface_position, surface_normal, buoyancy, linear_drag, angular_drag, Vec3::sZero(), Physics::Get().physics_system.GetGravity(), deltaTime);
	}
}

#if FT_DEBUG
void WaterScript::OnFixedUpdate(float fixedDeltaTime)
{
	using namespace JPH;
	auto tr = GetGameObject().GetComponent<WorldTransform>();
	// Draw the water surface
	const float step = 1.0f;
	for (float z = tr.position.z - tr.scale.z/2.0f; z < tr.position.z + tr.scale.z / 2.0f; z += step)
	{
		RVec3 p1 = _water->GetWaterSurfacePosition(RVec3(-tr.scale.x, 0, z));
		RVec3 p2 = _water->GetWaterSurfacePosition(RVec3(-tr.scale.x, 0, z + step));
		RVec3 p3 = _water->GetWaterSurfacePosition(RVec3(tr.scale.x, 0, z));
		RVec3 p4 = _water->GetWaterSurfacePosition(RVec3(tr.scale.x, 0, z + step));
		Physics::GetDebugRenderer()->DrawTriangle(p1, p2, p3, Color::sBlue);
		Physics::GetDebugRenderer()->DrawTriangle(p2, p4, p3, Color::sBlue);
	}
}
#endif

void WaterScript::OnLateUpdate(float deltaTime)
{
}

JPH::RVec3 Water::GetWaterSurfacePosition(JPH::RVec3Arg inXZPosition) const
{
	using namespace std::chrono;
	using namespace JPH;
	auto timePoint = Timer::Clock::now();
	auto mTime = time_point_cast<duration<double>>(timePoint);
	float t = mTime.time_since_epoch().count();
	return RVec3(inXZPosition.GetX(), cMinWaterHeight + Sin(waveLength * float(inXZPosition.GetZ()) + waveFrequency * t) * (cMaxWaterHeight - cMinWaterHeight), inXZPosition.GetZ());
}


Water::Water(Vector3 pos, EulerAngles rot, Vector3 scale, float waveAmplitude) {
	using namespace JPH;

	Scene& scene = Game::GetScene();

	_water = scene.CreateGameObject("Water");
	_water.AddComponent<Transform>( pos, rot, scale);
	_water.AddComponent<StaticMesh>("./resources/meshes/cube.fbx");

	ShapeRefC boxShape = BoxShapeSettings(Math::vector_cast<Vec3>(scale)).Create().Get();
	BodyCreationSettings water_sensor(
		boxShape,
		Math::vector_cast<Vec3>(pos),
		Math::vector_cast<Quat>(EulerToQuaternion(rot)),
		EMotionType::Static, ObjectLayers::WATER);
	water_sensor.mIsSensor = true;

	// Create water sensor. We use this to detect which bodies entered the water (in this sample we could have assumed everything is in the water)
	_water.AddComponent<RigidBody>(water_sensor, _water, EActivation::DontActivate);
	_bodyId = _water.GetComponent<RigidBody>().physicBody->bodyId;
	_water.AddScript<WaterScript>(this);

	cMinWaterHeight = pos.y + scale.y / 2 - waveAmplitude / 2.0f;
	cMaxWaterHeight = pos.y + scale.y / 2 + waveAmplitude / 2.0f;

}