#pragma once

#include "Frost.h"

using namespace Frost;

class Bullet
{
public:
	Bullet(Transform transform);
	~Bullet();
	void FixedUpdate(float deltaTime);

	bool IsExpired() const;

private:
	GameObject::Id _bullet;
	JPH::BodyID _bodyID;
	Timer _lifetimeTimer;
};

