#pragma once

#include "Frost.h"

namespace GameLogic
{
    class PlayerCamera : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
    };
} // namespace GameLogic