#include <Frost.h>
#include <Frost/Core/EntryPoint.h>
#include <iostream>

#include "MainLayer.h"

class Lab : public Frost::Application
{
public:
	Lab(Frost::ApplicationEntryPoint entryPoint) : Frost::Application{ entryPoint }
	{
		PushLayer(new MainLayer());
	}

	~Lab()
	{
	}
};

Frost::Application* Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
	entryPoint.title = "Lab";
	return new Lab(entryPoint);
}
