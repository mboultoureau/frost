#include "CheckpointScript.h"
#include "Frost.h"
#include "Frost/Debugging/Logger.h"
#include "Frost/Scene/Components/RigidBody.h"
#include "Frost/Physics/Physics.h"
#include "Physics/PhysicLayer.h"

using namespace Frost;

namespace GameLogic
{
    void CheckpointScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
    {
        auto layer1 = params.inBody1.GetObjectLayer();
        auto layer2 = params.inBody2.GetObjectLayer();

        if (layer1 == ObjectLayers::PLAYER)
        {
            GameObject playerObject = GameObject(static_cast<entt::entity>(params.inBody1.GetUserData()), GetScene());
            for (auto& Data : GameState::Get().GetPlayersData())
            {
                if (Data.playerObject == playerObject)
                {
                    auto& playerData = GameState::Get().GetPlayerData(playerObject);
                    auto MaxCheckpoint = GameState::Get().GetCheckpointsCount();
                    if (_checkpointID == 1 && playerData.checkpointNumber == MaxCheckpoint)
                    {
                        playerData.checkpointNumber = 1;
                    }
                    else if (_checkpointID + 1 == playerData.checkpointNumber)
                    {
                        playerData.checkpointNumber = _checkpointID;
                    }
                }
            }
        }
        else if (layer2 == ObjectLayers::PLAYER)
        {
            GameObject playerObject = GameObject(static_cast<entt::entity>(params.inBody2.GetUserData()), GetScene());
            for (auto& Data : GameState::Get().GetPlayersData())
            {
                if (Data.playerObject == playerObject)
                {
                    auto& playerData = GameState::Get().GetPlayerData(playerObject);
                    auto MaxCheckpoint = GameState::Get().GetCheckpointsCount();
                    if (_checkpointID == 1 && playerData.checkpointNumber == MaxCheckpoint)
                    {
                        playerData.checkpointNumber = 1;
                    }
                    else if (_checkpointID + 1 == playerData.checkpointNumber)
                    {
                        playerData.checkpointNumber = _checkpointID;
                    }
                }
            }
        }
    }
} // namespace GameLogic