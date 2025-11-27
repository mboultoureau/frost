#include "Shapes.h"
#include "../Game.h"
#include "Frost.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Shapes::Shapes()
{
    auto& scene = Game::GetScene();

    // Create a cube
    auto cube = scene.CreateGameObject("Cube");
    cube.AddComponent<Transform>(Vector3{ -2.0f, 0.0f, 0.0f });
    cube.AddComponent<StaticMesh>(ModelFactory::CreateCube(1.0f));

    auto sphere = scene.CreateGameObject("Sphere");
    sphere.AddComponent<Transform>(Vector3{ 2.0f, 0.0f, 0.0f });
    sphere.AddComponent<StaticMesh>(ModelFactory::CreateSphere(1.0f));

    auto plane = scene.CreateGameObject("Plane");
    plane.AddComponent<Transform>(Vector3{ -2.0f, 0.0f, 0.0f });
    plane.AddComponent<StaticMesh>(ModelFactory::CreatePlane(1.0f));
}
