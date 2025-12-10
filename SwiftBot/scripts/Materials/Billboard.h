#pragma once

#include <Frost.h>

namespace GameLogic
{

    class BillboardScript : public Frost::Scripting::Script
    {
    public:
        BillboardScript() = default;

        void OnUpdate(float deltaTime) override;

        void OnCreate() override;
    };
} // namespace GameLogic