#include "PlayerScript.h"
#include "../../Physics/PhysicsLayer.h"

using namespace Frost;

void
PlayerScript::OnPreFixedUpdate(float deltaTime)
{
    auto vehiclePair = player->GetCurrentVehicle();
    vehicle = vehiclePair.second;
    type = vehiclePair.first;
    // player->SetIsInWater(false);
    Physics::ActivateBody(player->GetBodyID());

    SelectedImageManager* manager = player->GetImageManager();

    // Switch to previous vehicle
    if (Input::GetKeyboard().IsKeyPressed(
            K_Q)) // ||
                  // Input::GetGamepad(gamepadId).IsButtonPressed(Gamepad::BUTTON_LEFT_SHOULDER))
    {
        auto prevVehicle =
            static_cast<Player::VehicleType>((type - 1 + player->GetVehicleNumber()) % player->GetVehicleNumber());
        player->SetPlayerVehicle(prevVehicle);
        manager->ChangeImageLeft();
        return;
    }
    // Switch to next vehicle
    if (Input::GetKeyboard().IsKeyPressed(K_E))
    {
        auto nextVehicle =
            static_cast<Player::VehicleType>((type + 1 + player->GetVehicleNumber()) % player->GetVehicleNumber());
        player->SetPlayerVehicle(nextVehicle);
        manager->ChangeImageRight();
        return;
    }

    vehicle->OnBrake(deltaTime, Input::GetKeyboard().IsKeyDown(K_SHIFT));

    float leftRightInput = 0;
    if (Input::GetKeyboard().IsKeyDown(K_A))
        leftRightInput -= 1;
    if (Input::GetKeyboard().IsKeyDown(K_D))
        leftRightInput += 1;
    // TODO gamepad
    vehicle->OnLeftRightInput(deltaTime, leftRightInput);

    float upDownInput = 0;
    if (Input::GetKeyboard().IsKeyDown(K_S))
        upDownInput -= 1;
    if (Input::GetKeyboard().IsKeyDown(K_W))
        upDownInput += 1;
    vehicle->OnAccelerateInput(deltaTime, upDownInput);

    vehicle->OnSpecial(deltaTime, Input::GetKeyboard().IsKeyDown(K_SPACE));

    vehicle->OnPreFixedUpdate(deltaTime);
};

void
PlayerScript::OnUpdate(float deltaTime)
{
    if (player->transitionTimer.GetDuration() > 500ms)
    {
        player->transitionRenderer.SetActive(false);
        player->transitionTimer.Pause();
    }

    player->GetCurrentVehicle().second->OnUpdate(deltaTime);
};

void
PlayerScript::OnLateUpdate(float deltaTime)
{
    player->GetCurrentVehicle().second->OnLateUpdate(deltaTime);
};

void
PlayerScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime)
{
    // TODO gamepad : vibration manette
    player->GetCurrentVehicle().second->OnCollisionEnter(params, deltaTime);
};

void
PlayerScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime)
{
    auto layer1 = Physics::GetBodyInterface().GetObjectLayer(params.inBody1.GetID());
    auto layer2 = Physics::GetBodyInterface().GetObjectLayer(params.inBody2.GetID());
    if (layer1 == ObjectLayers::WATER || layer2 == ObjectLayers::WATER)
    {
        player->SetIsInWater(true);
    }

    player->GetCurrentVehicle().second->OnCollisionStay(params, deltaTime);
};

// Warning : params may contains bodies that are not valid at the moment
void
PlayerScript::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime)
{
    auto scene = GetScene();
    auto go1 = scene->GetGameObjectFromId(params.first);
    if (go1.HasComponent<RigidBody>())
    {
        auto bodyId1 = go1.GetComponent<RigidBody>().physicBody->bodyId;
        auto layer1 = Physics::GetBodyInterface().GetObjectLayer(bodyId1);
        if (layer1 == ObjectLayers::WATER)
        {
            player->SetIsInWater(false);
        }
    }

    auto go2 = scene->GetGameObjectFromId(params.second);
    if (go2.HasComponent<RigidBody>())
    {
        auto bodyId2 = go2.GetComponent<RigidBody>().physicBody->bodyId;
        auto layer2 = Physics::GetBodyInterface().GetObjectLayer(bodyId2);
        if (layer2 == ObjectLayers::WATER)
        {
            player->SetIsInWater(false);
        }
    }

    player->GetCurrentVehicle().second->OnCollisionExit(params, deltaTime);
};
