#include "Moto.h"
#include "../Game.h"

#include "Frost/Scene/Components/Script.h"
#include "Frost/Scene/Components/StaticMesh.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"
#include "Frost/Utils/Math/Angle.h"
#include "Frost/Utils/Math/Vector.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

class RotateMoto : public Script
{
public:
    void OnUpdate(float deltaTime) override
    {
        auto& transform = GetGameObject().GetComponent<Transform>();
        transform.Rotate({ 0.0f, 1.0f * deltaTime, 0.0f });
    }
};

Moto::Moto()
{
    Scene& _scene = Game::GetScene();

    _moto = _scene.CreateGameObject("Moto");
    _moto.AddComponent<Transform>(
        Math::Vector3{ 0.0f, 5.0f, 5.0f }, Math::EulerAngles{ 0.0f, 0.0f, 0.0_deg }, Math::Vector3{ 1.0f, 1.0f, 1.0f });
    _moto.AddComponent<StaticMesh>(MeshSourceFile{ "./resources/meshes/moto.glb" });
    _moto.AddScript<RotateMoto>();
}