#include "CheckPoint.h"
#include "../Physics/PhysicsLayer.h"
#include "../Game.h"

#include "Frost/Scene/Components/RigidBody.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include <algorithm>
#include <cmath>
#include <DirectXMath.h>
#include <Frost/Scene/Components/GameObjectInfo.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

const float CHECKPOINT_SIZE = 5.0f;

class CheckPointScript : public Script {
private: 
	CheckPoint* checkPoint;
public:
	CheckPointScript(CheckPoint* checkpoint) : checkPoint{checkpoint}
	{
	}
	virtual void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override
	{
		checkPoint->ReinitializeChildrenPhysics();

		auto parents = checkPoint->GetParent();

		if (parents.empty()) {
			checkPoint->DestroyGameObject();
		}
		else {
			for (auto parent : parents) {
				parent->DeleteChildrenPhysics();
			}
		}
	
		CheckPoint::lastCheckPoint = GetGameObject();
	}

};

CheckPoint::CheckPoint(Transform::Vector3 startpos)
{
	Scene& scene = Game::GetScene();

	_checkpoint = scene.CreateGameObject("CheckPoint");
	scene.AddComponent<Transform>(
		_checkpoint,
		startpos,
		Transform::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f },
		Transform::Vector3{ 5.0f, 5.0f, 5.0f }
	);
	scene.AddComponent<WorldTransform>(_checkpoint);
}

void CheckPoint::FixedUpdate(float deltaTime) 
{
}


void CheckPoint::AddChild(std::shared_ptr<CheckPoint> child)
{
	_nextCheckPoints.push_back(child);
}

void CheckPoint::AddParent(std::shared_ptr<CheckPoint> parent)
{
	_prevCheckPoints.push_back(parent);
}

void CheckPoint::InitializePhysics()
{
	using namespace JPH;

	FT_ENGINE_ASSERT(_checkpoint != GameObject::InvalidId, "CheckPoint GameObject is invalid");
	Scene& scene = Game::GetScene();

	auto transform = scene.GetComponent<Transform>(_checkpoint);
	RVec3 position = Physics::Vector3ToJoltVector(transform->position);
	JPH::ShapeRefC boxShape = JPH::BoxShapeSettings(Vec3(5.0f, 5.0f, 5.0f)).Create().Get();
	BodyCreationSettings checkpoint_body_settings(boxShape, position, Quat::sIdentity(), EMotionType::Static, ObjectLayers::CHECKPOINT);

	checkpoint_body_settings.mGravityFactor = 0.0f;
	checkpoint_body_settings.mIsSensor = true;

	scene.AddComponent<RigidBody>(_checkpoint, checkpoint_body_settings, _checkpoint, EActivation::Activate);

}


void CheckPoint::ProcessInput(float deltaTime)
{
}


void CheckPoint::UpdatePhysics(float deltaTime)
{
}



void CheckPoint::ReinitializeChildrenPhysics()
{
	Scene& scene = Game::GetScene();

	for (std::shared_ptr<CheckPoint> child : _nextCheckPoints)
	{
		child->ActivatePhysics();	
	}
}


void CheckPoint::DeleteChildrenPhysics()
{
	using namespace JPH;
	Scene& scene = Game::GetScene();

	// Recuperation de l'interface Jolt
	JPH::BodyInterface* bodyInter = Physics::Get().body_interface;

	for (std::shared_ptr<CheckPoint> child : _nextCheckPoints)
	{
		GameObject::Id childId = child->_checkpoint;

		if (scene.GetComponent<Frost::Transform>(childId) == nullptr)
		{
			continue;
		}

		Frost::RigidBody* bodyComponent = scene.GetComponent<Frost::RigidBody>(childId);

		if (bodyComponent)
		{
			JPH::BodyID bodyId = bodyComponent->physicBody->bodyId;
			Physics::Get().mapJBodyGameObject.erase(bodyId);

			scene.RemoveComponent<Frost::RigidBody>(childId);

			scene.RemoveComponent<Frost::ModelRenderer>(childId);

			scene.RemoveComponent<CheckPointScript>(childId);
		}
	}
}

void CheckPoint::DestroyGameObject()
{
	using namespace JPH;
	Scene& scene = Game::GetScene();
	GameObject::Id id = _checkpoint;

	Frost::RigidBody* bodyComponent = scene.GetComponent<Frost::RigidBody>(id);

	if (bodyComponent)
	{
		JPH::BodyInterface* bodyInter = Physics::Get().body_interface;
		JPH::BodyID bodyId = bodyComponent->physicBody->bodyId;

		if (bodyId.IsInvalid() == false)
		{
			if (bodyInter->IsAdded(bodyId))
			{
				bodyInter->RemoveBody(bodyId);
			}
			bodyInter->DestroyBody(bodyId);
		}

	}

	scene.DestroyGameObject(id);
}
void CheckPoint::ActivatePhysics()
{
	Scene& scene = Game::GetScene();
	if (!scene.GetComponent<Frost::ModelRenderer>(_checkpoint))
	{
		scene.AddComponent<ModelRenderer>(_checkpoint, "./resources/meshes/sphere.fbx");
	}
	if (scene.GetComponent<Frost::RigidBody>(_checkpoint))
	{
		return;
	}
	InitializePhysics();
	scene.AddScript<CheckPointScript>(_checkpoint, this);
}


GameObject::Id CheckPoint::lastCheckPoint{ GameObject::InvalidId };