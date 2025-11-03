#pragma once

#include "Frost.h"

using namespace Frost;

class Goal
{
public:
	Goal();

private:
	GameObject::Id _goal;
	JPH::Body* _body;
};

