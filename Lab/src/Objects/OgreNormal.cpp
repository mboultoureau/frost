#include "OgreNormal.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class RotateOgreNormal : public Frost::Script
{
public:
    void OnFixedUpdate(float deltaTime) override
    {
        auto& transform = GetGameObject().GetComponent<Transform>();

        const float rotationSpeed = 1.0f;
        float angle = rotationSpeed * deltaTime;
        transform.Rotate(EulerAngles{ 0.0f, angle, 0.0f });
    }
};

OgreNormal::OgreNormal()
{
    Scene& _scene = Game::GetScene();

    _ogreNormal = _scene.CreateGameObject("Ogre Normal");
    _ogreNormal.AddComponent<Transform>(
        Vector3{ 2.0f, 0.0f, 5.0f }, EulerAngles{ 0.0f, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _ogreNormal.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });
    _ogreNormal.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/normal.fbx" });
    _ogreNormal.AddScript<RotateOgreNormal>();
}
