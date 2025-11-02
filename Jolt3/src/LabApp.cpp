#include "LabApp.h"

#include <Frost.h>
#include <Frost/Core/EntryPoint.h>
#include <iostream>

#include "MainLayer.h"

Lab::Lab(Frost::ApplicationEntryPoint entryPoint) : Frost::Application(entryPoint)
{
	_game = std::make_unique<Game>(this);
}

Lab::~Lab()
{
}


Frost::Application* Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
	entryPoint.title = "Lab";
	return new Lab(entryPoint);
}
