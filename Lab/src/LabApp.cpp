#include <Frost.h>

class Lab : public Frost::Application
{
public:
	Lab(Frost::ApplicationEntryPoint entryPoint) : Frost::Application{ entryPoint }
	{
	}

	~Lab()
	{
	}
};

Frost::Application* Frost::CreateApplication(ApplicationEntryPoint entryPoint)
{
	return new Lab(entryPoint);
}