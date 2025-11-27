#include "Text.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

Text::Text()
{
    Scene& _scene = Game::GetScene();

    _text = _scene.CreateGameObject("Text");
    _scene.AddComponent<Transform>(
        _text, Vector3{ 0.0f, 5.0f, 3.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _scene.AddComponent<WorldTransform>(_text, Vector3{ 0.0f, 0.0f, 0.0f });
    _scene.AddComponent<StaticMesh>(_text, "./resources/meshes/text.fbx");
}