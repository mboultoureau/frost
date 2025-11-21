#include "PlayerScript.h"


void PlayerScript::OnFixedUpdate(float deltaTime)
{
	auto vehiclePair = player->GetCurrentVehicle();
	vehicle = vehiclePair.second;
	type = vehiclePair.first;

	// Switch to previous vehicle
	if (Input::GetKeyboard().IsKeyPressed(K_Q))// || Input::GetGamepad(gamepadId).IsButtonPressed(Gamepad::BUTTON_LEFT_SHOULDER))
	{
		auto prevVehicle = static_cast<Player::VehicleType>((type - 1 + player->GetVehicleNumber()) % player->GetVehicleNumber());
		player->SetPlayerVehicle(prevVehicle);
		return;
	}
	// Switch to next vehicle
	if (Input::GetKeyboard().IsKeyPressed(K_E))
	{
		auto nextVehicle = static_cast<Player::VehicleType>((type + 1 + player->GetVehicleNumber()) % player->GetVehicleNumber());
		player->SetPlayerVehicle(nextVehicle);
		return;
	}


	vehicle->OnFixedUpdate(deltaTime);

	if (Input::GetKeyboard().IsKeyPressed(K_SHIFT))
	{
		vehicle->OnBrake(deltaTime);
	}

	float leftRightInput = 0;
	if (Input::GetKeyboard().IsKeyPressed(K_A)) leftRightInput -= 1;
	if (Input::GetKeyboard().IsKeyPressed(K_D)) leftRightInput += 1;
	// TODO gamepad
	vehicle->OnLeftRightInput(deltaTime, leftRightInput);


	float upDownInput = 0;
	if (Input::GetKeyboard().IsKeyPressed(K_W)) upDownInput -= 1;
	if (Input::GetKeyboard().IsKeyPressed(K_S)) upDownInput += 1;
	vehicle->OnAccelerateInput(deltaTime, upDownInput);


	if (Input::GetKeyboard().IsKeyPressed(K_SHIFT))
	{
		vehicle->OnSpecial(deltaTime);
	}
};

void PlayerScript::OnUpdate(float deltaTime) {
	if (player->transitionTimer.GetDuration() > 500ms) {
		player->transitionRenderer.SetActive(false);
		player->transitionTimer.Pause();
	}

	player->GetCurrentVehicle().second->OnUpdate(deltaTime);
};

void PlayerScript::OnLateUpdate(float deltaTime) {
	player->GetCurrentVehicle().second->OnLateUpdate(deltaTime);
};

void PlayerScript::OnCollisionEnter(BodyOnContactParameters params, float deltaTime) {
	// TODO gamepad : vibration manette
	player->GetCurrentVehicle().second->OnCollisionEnter(params, deltaTime);
};

void PlayerScript::OnCollisionStay(BodyOnContactParameters params, float deltaTime) {
	player->GetCurrentVehicle().second->OnCollisionStay(params, deltaTime);
};

// Warning : params may contains bodies that are not valid at the moment
void PlayerScript::OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) {
	player->GetCurrentVehicle().second->OnCollisionExit(params, deltaTime);
};

