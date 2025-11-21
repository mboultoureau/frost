#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

#include "../Player.h"

using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;

class Vehicle
{
public:

	struct RendererParameters {
		std::string name;
		std::string modelPath;
		Vector3 localPosition = Vector3{ 0, 0, 0 };
		EulerAngles localRotation = EulerAngles{ 0, 0, 0 };
		Vector3 localScale = Vector3{ 0, 0, 0 };

		RendererParameters(std::string name, std::string modelPath, Vector3 pos, EulerAngles rot, Vector3 scale)
			: name{ name }, modelPath{ modelPath }, localPosition{ pos }, localRotation{ rot }, localScale{ scale }
		{ }
	};

	// Called after constructor, so you have gameobject and ecs that are accessible
	Vehicle(Player* playerManager, RendererParameters rendererParams) : _player{ playerManager }, _scene{ _player->GetScene() }
	{
		_gameObjectRenderer = _scene->CreateGameObject("Vehicle Renderer : " + rendererParams.name, playerManager->GetPlayerID());
		_gameObjectRenderer.AddComponent<Transform>(
			rendererParams.localPosition,
			rendererParams.localRotation,
			rendererParams.localScale);
		_gameObjectRenderer.AddComponent<WorldTransform>();
		_gameObjectRenderer.AddComponent<StaticMesh>(rendererParams.modelPath);
	}

	/*======= Virtual event functions ========= */

	virtual void OnUpdate(float deltaTime) {};
	virtual void OnFixedUpdate(float fixedDeltaTime) {};
	virtual void OnLateUpdate(float deltaTime) {}

	virtual void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) {}
	virtual void OnCollisionStay(BodyOnContactParameters params, float deltaTime) {}
	// Warning : params may contains bodies that are not valid at the moment
	virtual void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) {}

	virtual void OnLeftRightInput(float deltaTime, float leftRightInput) = 0;
	virtual void OnAccelerateInput(float deltaTime, float upDownInput) = 0;
	virtual void OnBrake(float deltaTime) = 0;
	virtual void OnSpecial(float deltaTime) {};

	/*======= public interface ========= */

	GameObject GetModelRendererObject() { return _gameObjectRenderer; };
	void RenderMesh(bool shouldRender) {
		_gameObjectRenderer.SetActive(shouldRender);
	}
	JPH::BodyID GetBodyID() const { return _bodyId; };

	virtual JPH::BodyID Appear() = 0;
	virtual void Disappear() = 0;

protected:
	Player* _player;
	GameObject _gameObjectRenderer;
	Scene* _scene;
	JPH::BodyID _bodyId;
	Timer _fireTimer;
};

