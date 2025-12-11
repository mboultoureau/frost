#include "Looping/Looping.h"

using namespace Frost;
using namespace Frost::Component;

namespace GameLogic
{
    void Looping::OnCreate()
    {
        auto& staticMesh = GetGameObject().AddComponent<Frost::Component::StaticMesh>(MeshSourceFile{});
        staticMesh.SetModel(std::make_shared<Model>("./assets/Prefabs/Looping/Looping.glb"));

        if (!staticMesh.GetModel() || !staticMesh.GetModel()->HasMaterials())
        {
            FT_ENGINE_WARN(
                "DisableBackFaceCulling script attached to a GameObject without a valid StaticMesh or materials.");
            return;
        }
        staticMesh.GetModel()->GetMaterials()[0].backFaceCulling = false;
    }
} // namespace GameLogic