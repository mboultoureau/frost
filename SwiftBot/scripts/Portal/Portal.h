#pragma once

#include <Frost.h>

namespace GameLogic
{
    class Portal : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;

    private:
        void _Warp(Frost::GameObject playerObject);

    public:
        Frost::GameObject _linkedPortal;
        Frost::GameObject _triggerZone;

        bool _canTeleport = true;
    };

} // namespace GameLogic