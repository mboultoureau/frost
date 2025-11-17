#include "Frost/Scene/Systems/PhysicSystem.h"
#include "Frost/Scene/ECS/Component.h"
#include <Frost/Scene/Components/Meta.h>
#include "Frost/Scene/ECS/ECS.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include "Frost/Physics/Physics.h"

#include "Frost/Scene/Components/RigidBody.h"

using namespace Frost::Component;

class RigidBodyCannotHaveParents {};

Frost::PhysicSystem::PhysicSystem()
{
}

#pragma region Updates
void Frost::PhysicSystem::FixedUpdate(ECS& ecs, float deltaTime)
{
	Physics& physic = Physics::Get();

	physic.UpdatePhysics();

	UpdateAllJoltBodies(ecs, deltaTime);

	//Awake and sleep
	_HandleAwakeVector(ecs, deltaTime);
	_HandleSleepVector(ecs, deltaTime);
	_HandleOnCollisionEnterVector(ecs, deltaTime);
	_HandleOnCollisionStayVector(ecs, deltaTime);
	_HandleOnCollisionRemovedVector(ecs, deltaTime);
}

void Frost::PhysicSystem::LateUpdate(ECS& ecs, float deltaTime)
{
#ifdef FT_DEBUG
	Physics::DrawDebug();
#endif
}

//initialize jph::bodies and update ecs::transforms
void Frost::PhysicSystem::UpdateAllJoltBodies(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	auto rbodies = ecs.GetComponents<RigidBody>();
	auto goIds = ecs.GetIndexMap<RigidBody>();

	for (int i = 0; i < rbodies->data.size(); i++) {
		auto goId = goIds[i];
		auto rbody = ecs.GetComponent<RigidBody>(goId);


		auto jBodyPos = Physics::Get().body_interface->GetPosition(rbody->physicBody->bodyId);
		auto jBodyRot = Physics::Get().body_interface->GetRotation(rbody->physicBody->bodyId);

		auto transform = ecs.GetComponent<Transform>(goId);
		auto parent = ecs.GetComponent<Meta>(goId)->parentId;

		transform->position = Physics::JoltVectorToVector3(jBodyPos);

		JPH::Vec3 rotationRadians = jBodyRot.GetEulerAngles();
		transform->rotation.x = jBodyRot.GetX();
		transform->rotation.y = jBodyRot.GetY();
		transform->rotation.z = jBodyRot.GetZ();
		transform->rotation.w = jBodyRot.GetW();
	}
}

#pragma endregion

#pragma region Vector handling
void Frost::PhysicSystem::_HandleAwakeVector(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	std::for_each(physics.bodiesOnAwake.begin(), physics.bodiesOnAwake.end(), [&](const auto& params) {
		auto goId = physics.GetObjectID(params.inBodyID);
		auto scriptables = ecs.GetComponent<Scriptable>(goId);
		if (scriptables == nullptr) return;

		auto& scripts = scriptables->_scripts;

		std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
			script->OnAwake(deltaTime);
		});
	});
	physics.bodiesOnAwake.clear();
}

void Frost::PhysicSystem::_HandleSleepVector(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	std::for_each(physics.bodiesOnSleep.begin(), physics.bodiesOnSleep.end(), [&](const auto& params) {
		auto goId = physics.GetObjectID(params.inBodyID);
		auto scriptables = ecs.GetComponent<Scriptable>(goId);
		if (scriptables == nullptr) return;

		auto& scripts = scriptables->_scripts;

		std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
			script->OnSleep(deltaTime);
		});
	});
	physics.bodiesOnSleep.clear();
}

void Frost::PhysicSystem::_HandleOnCollisionEnterVector(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	std::for_each(physics.bodiesOnCollisionEnter.begin(), physics.bodiesOnCollisionEnter.end(), [&](const auto& params) {
		// Process id 1
		auto goId1 = physics.GetObjectID(params.inBody1);
		auto scriptables = ecs.GetComponent<Scriptable>(goId1);
		if (scriptables)
		{
			auto& scripts = scriptables->_scripts;
			std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
				script->OnCollisionEnter(params, deltaTime);
			});
		}

		//process id2
		auto goId2 = physics.GetObjectID(params.inBody2);
		scriptables = ecs.GetComponent<Scriptable>(goId2);
		if (scriptables)
		{
			auto& scripts = scriptables->_scripts;
			std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
				script->OnCollisionEnter(params, deltaTime);
			});
		}
	});
	physics.bodiesOnCollisionEnter.clear();
}

void Frost::PhysicSystem::_HandleOnCollisionStayVector(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();
	std::for_each(physics.bodiesOnCollisionStay.begin(), physics.bodiesOnCollisionStay.end(), [&](const auto& params) {
		// Process id 1
		auto goId1 = physics.GetObjectID(params.inBody1);
		auto scriptables = ecs.GetComponent<Scriptable>(goId1);
		if (scriptables) {
			auto& scripts = scriptables->_scripts;
			std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
				script->OnCollisionStay(params, deltaTime);
			});
		}


		//process id2
		auto goId2 = physics.GetObjectID(params.inBody2);
		scriptables = ecs.GetComponent<Scriptable>(goId2);
		if (scriptables) {
			auto& scripts = scriptables->_scripts;
			std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
				script->OnCollisionStay(params, deltaTime);
			});
		}

		physics.currentFrameBodyIDsOnCollisionStay.emplace(std::pair<Frost::GameObject::Id, Frost::GameObject::Id>( goId1, goId2 ));
	});
	physics.bodiesOnCollisionStay.clear();
}

void Frost::PhysicSystem::_HandleOnCollisionRemovedVector(ECS& ecs, float deltaTime)
{
	auto& physics = Physics::Get();

	for (auto params : physics.lastFrameBodyIDsOnCollisionStay) {
		// Check if idPair is not in current frame
		if (physics.currentFrameBodyIDsOnCollisionStay.find(params) == physics.currentFrameBodyIDsOnCollisionStay.end()) {
			// Process id 1
			auto goId1 = params.first;
			auto scriptables = ecs.GetComponent<Scriptable>(goId1);
			if (scriptables) {
				auto& scripts = scriptables->_scripts;
				std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
					script->OnCollisionExit(params, deltaTime);
				});
			}

			//process id2
			auto goId2 = params.second;
			scriptables = ecs.GetComponent<Scriptable>(goId2);
			if (scriptables) {
				auto& scripts = scriptables->_scripts;
				std::for_each(scripts.begin(), scripts.end(), [&](auto& script) {
					script->OnCollisionExit(params, deltaTime);
				});
			}
		}
	}

	physics.lastFrameBodyIDsOnCollisionStay = physics.currentFrameBodyIDsOnCollisionStay;
	physics.currentFrameBodyIDsOnCollisionStay.clear();
}

#pragma endregion
