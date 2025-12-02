#include "HierarchyTest.h"
#include "../Game.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class PivotScript : public Frost::Script
{
public:
    void OnFixedUpdate(float deltaTime) override
    {
        using namespace DirectX;
        auto& transform = GetGameObject().GetComponent<Transform>();
        XMFLOAT4 currentRot = vector_cast<XMFLOAT4>(transform.rotation);
        XMVECTOR deltaRot = XMQuaternionRotationRollPitchYaw(0, deltaTime, 0);
        XMVECTOR qCurrent = XMLoadFloat4(&currentRot);
        XMVECTOR qResult = XMQuaternionMultiply(qCurrent, deltaRot);
        XMVECTOR qResultGlobal = XMQuaternionMultiply(deltaRot, qCurrent);
        qResult = XMQuaternionNormalize(qResult);
        transform.rotation = vector_cast<Vector4>(qResult);
    }
};

HierarchyTest::HierarchyTest()
{
    Scene& _scene = Game::GetScene();

    _parent = _scene.CreateGameObject("Parent");
    _parent.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/sphere.fbx" });
    _parent.AddScript<PivotScript>();

    _child = _scene.CreateGameObject("Child", _parent);
    _child.AddComponent<Transform>(
        Vector3{ 0.0f, 0.0f, 5.0f }, EulerAngles{ -90.0_deg, 0.0f, 0.0f }, Vector3{ 1.0f, 1.0f, 1.0f });
    _child.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/cube.fbx" });
}
