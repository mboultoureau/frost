#include "LapCheckPoint.h"
#include "../Game.h"

void
LapCheckPoint::InitializePhysics()
{
    CheckPoint::InitializePhysics();
    Scene& scene = Game::GetScene();
    _checkpoint.AddScript<LapCheckPointScript>(this, _gamestate);
    CheckPoint::lastCheckPoint = _checkpoint;
}