#include "Ogre.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class RotateOgre : public Frost::Script
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

Ogre::Ogre()
{
    Scene& _scene = Game::GetScene();

    _ogre = _scene.CreateGameObject("Ogre");
    _ogre.AddComponent<Transform>(
        Vector3{ 0.0f, 0.0f, 5.0f }, EulerAngles{ 0.0_deg, 0.0_deg, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _ogre.AddComponent<WorldTransform>(Vector3{ 0.0f, 0.0f, 0.0f });
    _ogre.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/diffuse.fbx" });
    _ogre.AddScript<RotateOgre>();
}
