#pragma once

#include <Frost.h>
#include "../GameState/GameState.h"

namespace GameLogic
{
    class CheckpointScript : public Frost::Scripting::Script
    {
    public:
        CheckpointScript() = default;

        int _checkpointID = 0;

        void OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime) override;
    };
} // namespace GameLogic