#include "PhysicSystem.h"
#include "Frost/Scene/ECS/Component.h"
#include <Frost/Scene/Components/GameObjectInfo.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "Frost/Physics/Physics.h"

#include "Frost/Scene/Components/RigidBody.h"
#include <Frost/Scene/Components/WorldTransform.h>

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
	std::for_each(physic.bodiesOnAwake.begin(), physic.bodiesOnAwake.end(), [&](const auto& bodyId) {
		auto scriptables = ecs.GetComponent<Scriptable>(bodyId);
		if (scriptables == nullptr) return;

		auto& scripts = scriptables->_scripts;

		std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
			script->OnAwake();
		});
	});
	physic.bodiesOnAwake.clear();

	std::for_each(physic.bodiesOnSleep.begin(), physic.bodiesOnSleep.end(), [&](const auto& bodyId) {
		auto scriptables = ecs.GetComponent<Scriptable>(bodyId);
		if (scriptables == nullptr) return;

		auto& scripts = scriptables->_scripts;

		std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
			script->OnSleep();
		});
	});
	physic.bodiesOnSleep.clear();
	
	//Collisions
	auto v = physic.bodiesOnCollisionEnter;
	_HandleCollisionVector(physic.bodiesOnCollisionEnter, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionEnter(bodyIdPair);
	});
	
	_HandleCollisionVector(physic.bodiesOnCollisionStay, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionStay(bodyIdPair);
	});
	
	_HandleCollisionVector(physic.bodiesOnCollisionExit, ecs, deltaTime, [](auto& script, const auto& bodyIdPair) {
		script->OnCollisionExit(bodyIdPair);
	});
}

void Frost::PhysicSystem::LateUpdate(ECS& ecs, float deltaTime)
{
	Physics::DrawDebug();
}

//initialize jph::bodies and update ecs::transforms
void Frost::PhysicSystem::UpdateAllJoltBodies(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	auto rbodies = ecs.GetComponents<Frost::RigidBody>();
	auto goIds = ecs.GetIndexMap<RigidBody>();


	for (int i = 0; i < rbodies->data.size(); i++) {
		auto goId = goIds[i];
		auto rbody = ecs.GetComponent<Frost::RigidBody>(goId);

		auto jBodyPos = Physics::Get().body_interface->GetPosition(rbody->bodyId);
		auto jBodyRot = Physics::Get().body_interface->GetRotation(rbody->bodyId);

		auto transform = ecs.GetComponent<Transform>(goId);
		auto parent = ecs.GetComponent<Frost::GameObjectInfo>(goId)->parentId;

		transform->position = Physics::JoltVectorToVector3(jBodyPos);

		JPH::Vec3 rotationRadians = jBodyRot.GetEulerAngles();
		transform->rotation.x = jBodyRot.GetX();
		transform->rotation.y = jBodyRot.GetY();
		transform->rotation.z = jBodyRot.GetZ();
		transform->rotation.w = jBodyRot.GetW();
	}
}


template<typename Func>
void Frost::PhysicSystem::_HandleCollisionVector(Physics::PairIdCollisionVector& colVector, ECS& ecs, float deltaTime, Func&& callback)
{
	if (colVector.size() == 0) return;

	std::for_each(colVector.begin(), colVector.end(), [&](const auto& bodyIdPair) {
		auto* bodyScriptable = ecs.GetComponent<Scriptable>(bodyIdPair.first);
		if (bodyScriptable) {
			std::for_each(bodyScriptable->_scripts.begin(), bodyScriptable->_scripts.end(), [&](auto& script) {
				if (script->GetECS() == nullptr)
				{
					script->Initialize(bodyIdPair.first, &ecs);
				}
				callback(script, bodyIdPair.second);
			});
		}
		bodyScriptable = ecs.GetComponent<Scriptable>(bodyIdPair.second);
		if (bodyScriptable) {
			std::for_each(bodyScriptable->_scripts.begin(), bodyScriptable->_scripts.end(), [&](auto& script) {
				if (script->GetECS() == nullptr)
				{
					script->Initialize(bodyIdPair.second, &ecs);
				}
				callback(script, bodyIdPair.first);
			});
		}
		});
	colVector.clear();
}

