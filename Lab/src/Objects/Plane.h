#pragma once

#include "Frost.h"

#include <Jolt/Jolt.h>

class Plane
{
public:
	Plane();

private:
	Frost::GameObject::Id _plane;
	JPH::Body* _body;

private:
	void _SetupPhysics();
};

