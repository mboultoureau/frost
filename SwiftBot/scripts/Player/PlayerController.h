#pragma once

#include <Frost.h>

namespace GameLogic
{
    class PlayerController : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;
        void OnPreFixedUpdate(float deltaTime) override;

    public:
        Frost::GameObject _moto;
        Frost::GameObject _boat;
        Frost::GameObject _plane;

    private:
    };
} // namespace GameLogic
