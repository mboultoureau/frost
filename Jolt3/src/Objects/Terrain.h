#pragma once

#include "Frost.h"

using namespace Frost;

class Terrain
{
public:
	Terrain(Scene& scene);

private:
	GameObject::Id _terrain;
	JPH::Body* _body;
};

