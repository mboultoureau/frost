#pragma once

#include "Frost.h"

#include <Jolt/Jolt.h>

class HierarchyTest
{

public:
    HierarchyTest();

private:
    Frost::GameObject _parent;
    Frost::GameObject _child;
};
