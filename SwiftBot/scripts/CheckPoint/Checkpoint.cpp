#include "Checkpoint/Checkpoint.h"
#include "Physics/PhysicLayer.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void Checkpoint::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
    {
        // Get checkpoint number (Checkpoint_NUMBER)
        auto parent = GetGameObject().GetParent().GetParent();
        auto name = parent.GetComponent<Meta>().name;

        _checkpointID = std::stoi(name.substr(name.find_last_of('_') + 1));

        auto layer1 = params.inBody1.GetObjectLayer();
        auto layer2 = params.inBody2.GetObjectLayer();

        if (layer1 == ObjectLayers::PLAYER)
        {
            GameObject playerObject =
                GameObject(static_cast<entt::entity>(params.inBody1.GetUserData()), GetScene()).GetParent();
            for (auto& Data : GameState::Get().GetPlayersData())
            {
                if (Data.playerObject == playerObject)
                {
                    FT_INFO("Player {} hit Checkpoint {}", static_cast<uint32_t>(playerObject.GetId()), _checkpointID);
                    auto& playerData = GameState::Get().GetPlayerData(playerObject);
                    auto maxCheckpoint = GameState::Get().GetCheckpointsCount();
                    if (_checkpointID == 1 && playerData.checkpointNumber == maxCheckpoint)
                    {
                        playerData.lapsCompleted++;
                        playerData.checkpointNumber = 1;
                    }
                    else if (_checkpointID == playerData.checkpointNumber + 1)
                    {
                        playerData.checkpointNumber = _checkpointID;
                    }
                }
            }
        }
        else if (layer2 == ObjectLayers::PLAYER)
        {
            GameObject playerObject =
                GameObject(static_cast<entt::entity>(params.inBody2.GetUserData()), GetScene()).GetParent();
            for (auto& Data : GameState::Get().GetPlayersData())
            {
                FT_INFO("Player {} hit Checkpoint {}", static_cast<uint32_t>(playerObject.GetId()), _checkpointID);
                if (Data.playerObject == playerObject)
                {
                    auto& playerData = GameState::Get().GetPlayerData(playerObject);
                    auto maxCheckpoint = GameState::Get().GetCheckpointsCount();
                    if (_checkpointID == 1 && playerData.checkpointNumber == maxCheckpoint)
                    {
                        playerData.lapsCompleted++;
                        playerData.checkpointNumber = 1;
                    }
                    else if (_checkpointID == playerData.checkpointNumber + 1)
                    {
                        playerData.checkpointNumber = _checkpointID;
                    }
                }
            }
        }
    }
} // namespace GameLogic