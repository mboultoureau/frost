#pragma once
#include "CheckPoint.h"
#include "GameState.h"
#include "Frost.h"

using namespace Frost;

class LapCheckPointScript : public Script {
private:
	CheckPoint* checkPoint; 
	GameState& gamestate;  

public:
	LapCheckPointScript(CheckPoint* checkpoint, GameState& gs) : checkPoint{ checkpoint }, gamestate(gs)
	{
	}

	void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override
	{
		gamestate.IncreaseLap();

		checkPoint->ReinitializeChildrenPhysics();

		auto& parents = checkPoint->GetParent();

		if (parents.empty()) {
			checkPoint->DestroyGameObject();
		}
		else {
			for (auto& parent : parents) { 
				parent->DeleteChildrenPhysics();
			}
		}
	}
};

class LapCheckPoint : public CheckPoint {
public : 
	LapCheckPoint(Transform::Vector3 startpos, GameState& gamestate)
		: CheckPoint(startpos), _gamestate{ gamestate }
	{
	}

private: 
	GameState& _gamestate;
	void InitializePhysics() override;
};