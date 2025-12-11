#include "GrassLayer.h"
#include "GameState/GameState.h"
#include "Physics/PhysicLayer.h"

using namespace JPH;
using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void GrassLayer::OnCreate() {}

    void GrassLayer::OnUpdate(float deltaTime) {}

    void GrassLayer::OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime)
    {
        auto bodyId1 = params.inBody1.GetID();
        auto bodyId2 = params.inBody2.GetID();

        auto otherBodyId = (GetGameObject().GetComponent<RigidBody>().runtimeBodyID == bodyId1) ? bodyId2 : bodyId1;

        uint64_t userData = Physics::GetBodyInterface().GetUserData(otherBodyId);
        if (userData == 0)
            return;

        auto entityHandle = static_cast<entt::entity>(userData);
        GameObject playerObject(entityHandle, GetScene());

        if (playerObject.IsValid() && GameState::Get().IsPlayer(playerObject.GetParent()))
        {
            // Met à jour le GameState : le joueur est dans l'herbe
            GameState::Get().GetPlayerData(playerObject.GetParent()).isInGrass = true;
        }
    }

    void GrassLayer::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
    {
        auto grassHandle = GetGameObject().GetHandle();
        auto otherHandle = (params.first == grassHandle) ? params.second : params.first;

        GameObject otherObject = GetScene()->GetGameObjectFromId(otherHandle);

        if (otherObject.IsValid() && GameState::Get().IsPlayer(otherObject.GetParent()))
        {
            GameState::Get().GetPlayerData(otherObject.GetParent()).isInGrass = false;
        }
    }
} // namespace GameLogic