#pragma once

#include "Frost.h"

using namespace Frost;

class Terrain
{

public:
	Terrain();
	float heightScale = 1.0f;
	std::string filepath;

private:
	GameObject _terrain;
};


