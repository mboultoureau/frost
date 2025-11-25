#include "Boost.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

void BoostScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
	auto id1 = params.inBody1.GetID();
	auto id2 = params.inBody2.GetID();


	if (id1 == _bodyId) {
		Physics::GetBodyInterface().AddForce(id2, _dir);
		return;
	}
	Physics::GetBodyInterface().AddForce(id1, _dir);
}

Boost::Boost(Vector3 pos, EulerAngles rot, Vector3 scale, Frost::Math::Vector3 dir, float power)
{
	using namespace JPH;

	Scene& scene = Game::GetScene();

	boost = scene.CreateGameObject("Boost");
	boost.AddComponent<Transform>(pos, rot, scale);
	boost.AddComponent<StaticMesh>("./resources/meshes/cube.fbx");

	ShapeRefC boxShape = BoxShapeSettings(Math::vector_cast<Vec3>(scale)).Create().Get();
	BodyCreationSettings bodySett(
		boxShape,
		Math::vector_cast<Vec3>(pos),
		Math::vector_cast<Quat>(EulerToQuaternion(rot)),
		EMotionType::Static, ObjectLayers::BOOST);
	bodySett.mIsSensor = true;

	// Create water sensor. We use this to detect which bodies entered the water (in this sample we could have assumed everything is in the water)
	boost.AddComponent<RigidBody>(bodySett, boost, EActivation::DontActivate);
	boost.AddScript<BoostScript>(dir, power);
}
