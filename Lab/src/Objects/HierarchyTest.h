#pragma once

#include "Frost.h"

#include <Jolt/Jolt.h>

class HierarchyTest
{

public:
	HierarchyTest();

private:
	Frost::GameObject::Id _parent;
	Frost::GameObject::Id _child;

};

