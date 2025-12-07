#pragma once

#include <Frost.h>

namespace GameLogic
{
    class Rotate : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
    };
} // namespace GameLogic
