#include "PhysicSystem.h"
#include "Frost/Scene/ECS/Component.h"
#include <Frost/Scene/Components/GameObjectInfo.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "Frost/Physics/Physics.h"

class RigidBodyCannotHaveParents {};

Frost::PhysicSystem::PhysicSystem()
{
}

void Frost::PhysicSystem::FixedUpdate(ECS& ecs, float deltaTime)
{
	Physics& physic = Physics::Get();

	physic.UpdatePhysics();

	UpdateAllJoltBodies(ecs, deltaTime);

	//Awake and sleep
	auto& scriptables = ecs.GetDataArray<Scriptable>();

	std::for_each(physic.bodiesOnAwake.begin(), physic.bodiesOnAwake.end(), [&](const auto& bodyId) {
		auto& bodyScripts = scriptables[bodyId]._scripts;
		std::for_each(bodyScripts.begin(), bodyScripts.end(), [&](auto& script) {
			script->OnAwake();
			});
		});
	physic.bodiesOnAwake.clear();

	std::for_each(physic.bodiesOnSleep.begin(), physic.bodiesOnSleep.end(), [&](const auto& bodyId) {
		auto& bodyScripts = scriptables[bodyId]._scripts;
		std::for_each(bodyScripts.begin(), bodyScripts.end(), [&](auto& script) {
			script->OnSleep();
			});
		});
	physic.bodiesOnSleep.clear();
	
	//Collisions
	auto v = physic.bodiesOnCollisionEnter;
	_HandleCollisionVector(physic.bodiesOnCollisionEnter, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionEnter(bodyIdPair.second);
		});
	_HandleCollisionVector(physic.bodiesOnCollisionStay, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionStay(bodyIdPair.second);
		});
	_HandleCollisionVector(physic.bodiesOnCollisionExit, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionExit(bodyIdPair.second);
		});

	/*auto& collExitVect = physic.bodiesOnCollisionExit;
	if (collExitVect.size() > 0) {
		std::for_each(collExitVect.begin(), collExitVect.end(), [&](const auto& bodyIdPair) {
			auto* bodyScriptable = ecs.GetComponent<Scriptable>(bodyIdPair.first);
			if (bodyScriptable) {
				std::for_each(bodyScriptable->_scripts.begin(), bodyScriptable->_scripts.end(), [&](auto& script) {
					script->OnCollisionExit(bodyIdPair.second);
					});
				auto& map_JBId_GoId = Physics::Get().mapJBodyGameObject;
				std::erase_if(map_JBId_GoId, [&](auto pairJbGo) { return bodyIdPair.first == pairJbGo.second; });
			}
			});
		collExitVect.clear();
	}*/
}

//initialize jph::bodies and update ecs::transforms
void Frost::PhysicSystem::UpdateAllJoltBodies(ECS& ecs, float deltaTime)
{
	auto rbodies = ecs.GetComponents<Frost::RigidBody>();
	auto goIds = ecs.GetIndexMap<RigidBody>();

	for (int i = 0; i < rbodies->data.size(); i++) {
		auto goId = goIds[i];
		auto rbody = ecs.GetComponent<Frost::RigidBody>(goId);

		if (rbody->bodyId == JPH::BodyID()) {
			InitRigidBody(ecs, rbody, goId);
		}

		auto id = rbody->bodyId;
		auto jBodyPos = Physics::Get().body_interface->GetPosition(id);
		auto jBodyRot = Physics::Get().body_interface->GetRotation(id);
		
		auto transform = ecs.GetComponent<Transform>(goId);
		auto parent = ecs.GetComponent<Frost::GameObjectInfo>(goId)->parentId;

		if (parent == GameObject::InvalidId) {
			transform->position = Physics::JoltVectorToVector3(jBodyPos);

			auto wTransfRotTemp = jBodyRot;
			transform->rotation = Physics::JoltVectorToVector3(wTransfRotTemp.GetEulerAngles());
		}
		else throw RigidBodyCannotHaveParents();
	}
}

class RigidbodyCouldNotBeInitialized {};
void Frost::PhysicSystem::InitRigidBody(ECS& ecs, RigidBody* rb, GameObject::Id id)
{
	auto goInfo = ecs.GetComponent<GameObjectInfo>(id);
	if (goInfo->parentId != GameObject::InvalidId) throw RigidBodyCannotHaveParents();

	auto transform = ecs.GetComponent<Transform>(id);
	bool isActive = true;//ecs.GetActiveGameObjects());
	auto activationMode = isActive ? JPH::EActivation::Activate : JPH::EActivation::DontActivate; 

	auto test = Physics::Get()._rigidbodyFuturColliders;
	auto shapeRef = Physics::Get()._rigidbodyFuturColliders.at(goInfo->id);

	JPH::BodyCreationSettings bodySettings(
		shapeRef,
		Physics::Vector3ToJoltVector(transform->position),
		JPH::Quat::sEulerAngles(Physics::Vector3ToJoltVector(transform->rotation)),
		rb->motionType,
		rb->objectLayer);
	bodySettings.mUserData = id;
	auto Jid = Physics::Get().body_interface->CreateAndAddBody(bodySettings, activationMode);
	
	/*if (wTransform) {
		Physics::Vector3ToJoltVector(transform->position),
		JPH::Quat::sEulerAngles(Physics::Vector3ToJoltVector(transform->rotation)),
	}*/

	rb->bodyId = Jid;

	if(ecs.GetComponent<RigidBody>(id)->bodyId == JPH::BodyID()) throw RigidbodyCouldNotBeInitialized();
}



template<typename Func>
void Frost::PhysicSystem::_HandleCollisionVector(Physics::PairIdCollisionVector& colVector, ECS& ecs, float deltaTime, Func&& callback)
{
	if (colVector.size() == 0) return;

	std::for_each(colVector.begin(), colVector.end(), [&](const auto& bodyIdPair) {
		auto* bodyScriptable = ecs.GetComponent<Scriptable>(bodyIdPair.first);
		if (bodyScriptable) {
			std::for_each(bodyScriptable->_scripts.begin(), bodyScriptable->_scripts.end(), [&](auto& script) {
				callback(script, bodyIdPair);
				});
		}
		bodyScriptable = ecs.GetComponent<Scriptable>(bodyIdPair.second);
		if (bodyScriptable) {
			std::for_each(bodyScriptable->_scripts.begin(), bodyScriptable->_scripts.end(), [&](auto& script) {
				callback(script, bodyIdPair);
				});
		}
		});
	colVector.clear();
}



#pragma region to delete
/*
void Frost::PhysicSystem::UpdateColliderRigidbodyLinks(ECS& ecs)
{
	auto colliders = ecs.GetComponents<Collider>();
	auto& physics = Physics::Get();

	for (int i = 0; i < colliders->data.size(); i++) {
		auto collider = colliders->data[i];
		auto colliderInfo = ecs.GetComponent<GameObjectInfo>(colliders->indexToEntity[i]);
		auto colliderTransform = ecs.GetComponent<Transform>(colliders->indexToEntity[i]);
		auto shape = physics.GetColliderShapeRef(&collider);


		if (colliderInfo->parentId != collider.rigidbodyId) {
			if (collider.rigidbodyId != -1) {
				//remove shape in old body compound shape
				auto oldParentRigidBody = ecs.GetComponent<RigidBody>(collider.rigidbodyId);
				if (oldParentRigidBody)
				{
					physics.RemoveSubShapeFromBody((oldParentRigidBody->bodyId), collider.subShapeIndex, JPH::EActivation::Activate);
				}
			}
			auto newParentRigidBody = ecs.GetComponent<RigidBody>(colliderInfo->parentId);
			int index = physics.AddSubShapeToBody(
				(newParentRigidBody->bodyId),
				shape,
				JPH::EActivation::Activate,
				JPH::Vec3Arg{ colliderTransform->position.x, colliderTransform->position.y, colliderTransform->position.z },
				JPH::Quat::sEulerAngles(JPH::Vec3{ colliderTransform->rotation.x, colliderTransform->rotation.y, colliderTransform->rotation.z })
			);
			physics.body_interface->SetMotionType((newParentRigidBody->bodyId), newParentRigidBody->motionType, JPH::EActivation::Activate);
			if(index < 0) 
				collider.subShapeIndex = index;
			collider.rigidbodyId = colliderInfo->parentId;
			collider.hasShapeBeenModified = false;
			continue;
		}


		if (collider.hasShapeBeenModified) {
			auto parentRigidBody = ecs.GetComponent<RigidBody>(colliderInfo->parentId);

			physics.RemoveSubShapeFromBody((parentRigidBody->bodyId), collider.subShapeIndex, JPH::EActivation::Activate);
			int index = physics.AddSubShapeToBody(
				(parentRigidBody->bodyId),
				shape,
				JPH::EActivation::Activate,
				JPH::Vec3Arg{ colliderTransform->position.x, colliderTransform->position.y, colliderTransform->position.z },
				JPH::Quat::sEulerAngles(JPH::Vec3{ colliderTransform->rotation.x, colliderTransform->rotation.y, colliderTransform->rotation.z })
			);
			physics.body_interface->SetMotionType((parentRigidBody->bodyId), parentRigidBody->motionType, JPH::EActivation::Activate);
			if (index < 0)
				collider.subShapeIndex = index;
			collider.rigidbodyId = colliderInfo->parentId;
			collider.hasShapeBeenModified = false;
		}

		/*if (collider.isActivated) {
			// if(collider.parent n'a PAS la shape dans le compoundShape)
				// add shape dans parent body compound shape
		}
		else {
			// remove old shape in parent body compound shape
		}
	}
}*/
#pragma endregion