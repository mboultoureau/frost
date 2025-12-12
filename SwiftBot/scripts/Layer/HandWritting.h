#pragma once

#include <Frost.h>
#include <Jolt/Jolt.h>

namespace GameLogic
{
    class HandWritting : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;

        void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;
        void OnCollisionExit(std::pair<Frost::GameObject::Id, Frost::GameObject::Id> params, float deltaTime) override;
    };
} // namespace GameLogic