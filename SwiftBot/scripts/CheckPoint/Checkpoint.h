#pragma once

#include <Frost.h>
#include "GameState/GameState.h"

namespace GameLogic
{
    class Checkpoint : public Frost::Scripting::Script
    {
    public:
        Checkpoint() = default;

        int _checkpointID = 0;

        void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;
    };
} // namespace GameLogic