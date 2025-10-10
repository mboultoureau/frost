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
	entryPoint.title = "Lab";
	return new Lab(entryPoint);
}

int main()
{
	Frost::Scene scene;
}