#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>

using namespace Frost;

class CheckPoint
{
public:
	static GameObject::Id lastCheckPoint;

	CheckPoint(Transform::Vector3 startpos);
	void FixedUpdate(float deltaTime);
	void AddChild(std::shared_ptr<CheckPoint> child);
	void AddParent(std::shared_ptr<CheckPoint> parent);
	void ReinitializeChildrenPhysics();
	void DeleteChildrenPhysics();
	void DestroyGameObject();
	virtual void ActivatePhysics();
	std::vector<std::shared_ptr<CheckPoint>>& GetParent() { return _prevCheckPoints; };
private:
	std::vector<std::shared_ptr<CheckPoint>> _nextCheckPoints;
	std::vector<std::shared_ptr<CheckPoint>> _prevCheckPoints;
protected:
	GameObject::Id _checkpoint;

	virtual void InitializePhysics();
	void ProcessInput(float deltaTime);
	void UpdatePhysics(float deltaTime);
};



