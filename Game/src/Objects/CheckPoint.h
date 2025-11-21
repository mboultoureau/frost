#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>

using namespace Frost;

class CheckPoint
{
public:
	static GameObject lastCheckPoint;

	CheckPoint(Math::Vector3 startpos);
	void FixedUpdate(float deltaTime);
	void AddChild(std::shared_ptr<CheckPoint> child);
	void AddParent(std::shared_ptr<CheckPoint> parent);
	void ReinitializeChildrenPhysics();
	void DeleteChildrenPhysics();
	void DestroyGameObject();
	virtual void ActivatePhysics();
	std::vector<std::shared_ptr<CheckPoint>>& GetParent() { return _prevCheckPoints; };

	GameObject GetGameObjectId() const { return _checkpoint; }
private:
	std::vector<std::shared_ptr<CheckPoint>> _nextCheckPoints;
	std::vector<std::shared_ptr<CheckPoint>> _prevCheckPoints;
protected:
	GameObject _checkpoint;

	virtual void InitializePhysics();
	void ProcessInput(float deltaTime);
	void UpdatePhysics(float deltaTime);
};



