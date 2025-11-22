#pragma once
#include "Player.h"
#include "Vehicles/Bike.h"
#include "Vehicles/Plane.h"
#include "Frost/Scene/Components/Script.h"

class PlayerScript : public Script {
	friend class Vehicle;

	Player* player ;
	Vehicle* vehicle;
	Player::VehicleType type;

public:
	PlayerScript(Player* player) : player{player}
	{
	}

	~PlayerScript() 
	{
		player->GetCurrentVehicle().second->Disappear();
	}

	void OnUpdate(float deltaTime) override;

	void OnPreFixedUpdate(float fixedDeltaTime) override;

	void OnLateUpdate(float deltaTime) override;

	void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;

	void OnCollisionStay(BodyOnContactParameters params, float deltaTime) override;

	// Warning : params may contains bodies that are not valid at the moment
	void OnCollisionExit(std::pair<GameObject::Id, GameObject::Id> params, float deltaTime) override;
};