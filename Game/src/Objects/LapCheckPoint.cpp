#include "LapCheckPoint.h"
#include "../Game.h"

void LapCheckPoint::InitializePhysics()
{
    CheckPoint::InitializePhysics();
    Scene& scene = Game::GetScene();
    scene.AddScript<LapCheckPointScript>(_checkpoint, this, _gamestate);
}