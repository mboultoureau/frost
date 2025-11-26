#pragma once

#include "Frost.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Body/Body.h>
#include "Frost/Scene/Components/Script.h"



class Boost
{
public:
	Frost::GameObject boost;
	Boost(Frost::Math::Vector3 pos, Frost::Math::EulerAngles rot, Frost::Math::Vector3 scale, Frost::Math::Vector3 dir, float power);
};


class BoostScript : public Frost::Script
{
	friend class Vehicle;
	JPH::Vec3 _dir;
	JPH::BodyID _bodyId;

public:
	BoostScript(Frost::Math::Vector3 dir, float power) :
		_dir{ vector_cast<JPH::Vec3>(dir).Normalized() * power }
	{
	}
	void OnInitialize() override {
		_bodyId = GetGameObject().GetComponent<Frost::Component::RigidBody>().physicBody->bodyId;
	}
	void OnCollisionStay(Frost::BodyOnContactParameters params, float deltaTime) override;
};