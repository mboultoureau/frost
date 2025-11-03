#pragma once

#include "Frost.h"

using namespace Frost;

class Cargo
{
public:
	Cargo();

private:
	GameObject::Id _cargo;

	JPH::Body* _body;
};

