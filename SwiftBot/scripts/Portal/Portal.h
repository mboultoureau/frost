#pragma once

#include <Frost.h>

namespace GameLogic
{
    class Portal : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        virtual void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;

    private:
        void _Warp(Frost::GameObject playerObject);

    public:
        Frost::GameObject _linkedPortal;
        bool _canTeleport = true;
    };

} // namespace GameLogic