#include "Portal/Portal.h"
#include "GameState/GameState.h"
#include "Physics/PhysicLayer.h"

#include <Jolt/Physics/Body/BodyInterface.h>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void Portal::OnCreate()
    {
        // Check if parent portal exists
        if (!GetGameObject().GetParent().IsValid())
        {
            _canTeleport = false;
            return;
        }

        if (!GetGameObject().GetParent().GetParent().IsValid())
        {
            _canTeleport = false;
            return;
        }

        auto parentPortal = GetGameObject().GetParent().GetParent();
        auto name = parentPortal.GetComponent<Meta>().name;
        FT_INFO("Portal parent name: {}", name);

        std::string myName = name;
        std::string suffix_in = "_IN";
        std::string suffix_out = "_OUT";

        size_t pos = myName.rfind(suffix_in);
        if (pos != std::string::npos && pos == myName.length() - suffix_in.length())
        {
            _canTeleport = false;

            std::string targetName = myName;
            targetName.replace(pos, suffix_in.length(), suffix_out);

            _linkedPortal = GetScene()->FindGameObjectByName(targetName);

            if (_linkedPortal.IsValid())
            {
                FT_INFO("Portail d'entrée [{}] lié avec succès au portail de sortie [{}]", myName, targetName);
                _canTeleport = true;
            }
            else
            {
                FT_ERROR("Le portail d'entrée [{}] n'a pas pu trouver son portail de sortie nommé [{}] !",
                         myName,
                         targetName);
            }
        }
        else
        {
            _canTeleport = false;
        }
    }

    void Portal::OnCollisionEnter(Frost::BodyOnContactParameters params, float deltaTime)
    {
        auto portalBodyId = GetGameObject().GetComponent<RigidBody>().runtimeBodyID;
        auto otherBodyId = (portalBodyId == params.inBody1.GetID()) ? params.inBody2.GetID() : params.inBody1.GetID();

        uint64_t userData = Physics::GetBodyInterface().GetUserData(otherBodyId);
        if (userData == 0)
            return;

        auto entityHandle = static_cast<entt::entity>(userData);
        GameObject otherObject(entityHandle, GetScene());

        GameObject playerRoot = otherObject.GetParent();
        if (!playerRoot.IsValid() || !GameState::Get().IsPlayer(playerRoot))
        {
            return;
        }

        if (!_canTeleport || !_linkedPortal.IsValid())
        {
            return;
        }

        _Warp(playerRoot);
    }

    void Portal::_Warp(Frost::GameObject playerObject)
    {
        auto playerController = playerObject.GetChildByName("PlayerController");
        if (!playerController.IsValid())
        {
            FT_ERROR("Objet 'PlayerController' non trouvé comme enfant de l'objet joueur. Téléportation annulée.");
            return;
        }

        auto camera = playerObject.GetChildByName("Camera", true);
        if (!camera.IsValid())
        {
            FT_ERROR("Objet 'Camera' non trouvé dans la hiérarchie du joueur. Téléportation annulée.");
            return;
        }

        auto* scene = GetScene();
        auto& bodyInterface = Physics::Get().body_interface;

        auto portal1Transform = scene->GetComponent<WorldTransform>(GetGameObject());
        auto portal2Transform = scene->GetComponent<WorldTransform>(_linkedPortal);

        // Portal 2 +5 y relative position
        portal2Transform->position.y += 5;

        auto playerTransform = scene->GetComponent<WorldTransform>(playerController);
        auto playerRb = scene->GetComponent<RigidBody>(playerController);
        auto playerBodyId = playerRb->runtimeBodyID;

        auto cameraTransform = scene->GetComponent<WorldTransform>(camera);
        auto cameraRb = scene->GetComponent<RigidBody>(camera);
        auto cameraBodyId = cameraRb->runtimeBodyID;

        JPH::Vec3 playerSpeed = bodyInterface->GetLinearVelocity(playerBodyId);
        JPH::Vec3 cameraSpeed = bodyInterface->GetLinearVelocity(cameraBodyId);

        JPH::Quat portal1Rotation = Math::vector_cast<JPH::Quat>(portal1Transform->rotation);
        JPH::Quat portal2Rotation = Math::vector_cast<JPH::Quat>(portal2Transform->rotation);
        JPH::Quat playerRotation = bodyInterface->GetRotation(playerBodyId);
        JPH::Quat cameraRotation = bodyInterface->GetRotation(cameraBodyId);

        JPH::Quat flipY = JPH::Quat::sRotation(JPH::Vec3::sAxisY(), JPH::JPH_PI);
        JPH::Quat finalPortal2Rotation = portal2Rotation * flipY;

        JPH::Quat rotAInv = portal1Rotation.Conjugated();

        // Calcul de la nouvelle position du joueur
        JPH::Vec3 localPos = rotAInv * (Math::vector_cast<JPH::Vec3>(playerTransform->position) -
                                        Math::vector_cast<JPH::Vec3>(portal1Transform->position));
        JPH::Vec3 newPlayerPos =
            Math::vector_cast<JPH::Vec3>(portal2Transform->position) + finalPortal2Rotation * localPos;

        JPH::Quat localRot = rotAInv * playerRotation;
        JPH::Quat newPlayerRot = finalPortal2Rotation * localRot;

        JPH::Vec3 localVel = rotAInv * playerSpeed;
        JPH::Vec3 newPlayerSpeed = finalPortal2Rotation * localVel;

        bodyInterface->SetPositionAndRotation(playerBodyId, newPlayerPos, newPlayerRot, JPH::EActivation::Activate);
        bodyInterface->SetLinearVelocity(playerBodyId, newPlayerSpeed);

        JPH::Vec3 offsetLocal = rotAInv * (Math::vector_cast<JPH::Vec3>(cameraTransform->position) -
                                           Math::vector_cast<JPH::Vec3>(playerTransform->position));
        JPH::Vec3 offsetWorld = newPlayerRot * (rotAInv * (Math::vector_cast<JPH::Vec3>(cameraTransform->position) -
                                                           Math::vector_cast<JPH::Vec3>(playerTransform->position)));

        JPH::Quat camLocalRot = rotAInv * cameraRotation;
        JPH::Quat newCamRot = finalPortal2Rotation * camLocalRot;

        JPH::Vec3 camLocalVel = rotAInv * cameraSpeed;
        JPH::Vec3 newCamVel = finalPortal2Rotation * camLocalVel;

        JPH::Vec3 newCamPos = newPlayerPos + offsetWorld;

        bodyInterface->SetPositionAndRotation(cameraBodyId, newCamPos, newCamRot, JPH::EActivation::Activate);
        bodyInterface->SetLinearVelocity(cameraBodyId, newCamVel);

        if (GameState::Get().IsInitialized())
        {
            auto& playerData = GameState::Get().GetPlayerData(playerObject);
            playerData.forceSpecificCameraPos = true;
        }
    }
} // namespace GameLogic