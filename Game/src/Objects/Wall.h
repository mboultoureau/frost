#pragma once
#include "Frost.h"

using namespace Frost;

class Wall
{
public:
	Wall();

private:
	GameObject _wall;
	void _SetupPhysics();
};
