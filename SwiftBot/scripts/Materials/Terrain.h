#pragma once

#include <Frost.h>

namespace GameLogic
{
    class Terrain : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
    };
} // namespace GameLogic
