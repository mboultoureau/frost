#include "LabApp.h"

#include <Frost.h>
#include <Frost/Core/EntryPoint.h>
#include <iostream>

#include "MainLayer.h"
#include "Physics/PhysicsLayer.h"

Lab::Lab(Frost::ApplicationEntryPoint entryPoint) :
    Frost::Application(entryPoint),
    _physicsConfig{ new GameBroadPhaseLayerInterface(),
                    new GameObjectLayerPairFilter(),
                    new GameObjectVsBroadPhaseLayerFilter() }
{
    ConfigurePhysics(_physicsConfig);
}

Lab::~Lab() {}

void
Lab::OnApplicationReady()
{
    _game = std::make_unique<Game>(this);
}

Frost::Application*
Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
    entryPoint.title = Window::WindowTitle{ L"Lab" };
    return new Lab(entryPoint);
}
