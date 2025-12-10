#include "Billboard.h"
#include "Frost/Scene/Components/Transform.h"
#include "Frost/Scene/Components/WorldTransform.h"

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

namespace GameLogic
{
    void BillboardScript::OnUpdate(float deltaTime)
    {
        /*
        Vector3 cameraPosition = { 0.0f, 0.0f, 0.0f };

        auto mainLayer = Game::GetMainLayer();
        if (mainLayer)
        {
            auto player = mainLayer->GetPlayer();
            if (player)
            {
                auto& scene = Game::GetScene();
                auto playerCameraId = player->GetCamera()->GetCameraId();

                auto camWorldTransform = scene.GetComponent<WorldTransform>(playerCameraId);

                if (camWorldTransform)
                {
                    cameraPosition = camWorldTransform->position;
                }
            }
        }

        // 2. Appliquer la rotation
        if (cameraPosition.length() > 0.0f)
        {
            FaceCamera(cameraPosition);
        }

        */
    }

    void BillboardScript::FaceCamera(const Vector3& cameraPosition)
    {
        GameObject currentObject = GetGameObject();
        if (!currentObject.HasComponent<Transform>())
        {
            FT_ASSERT(false, "Billboard GameObject missing Transform component!");
            return;
        }

        Transform& transform = currentObject.GetComponent<Transform>();

        Vector3 billboardPosition = transform.position;

        Vector3 lookDirection = billboardPosition - cameraPosition;
        lookDirection.y = 0.0f;
        lookDirection =
            lookDirection /
            std::sqrt((std::pow(lookDirection.x, 2) + std::pow(lookDirection.z, 2) + std::pow(lookDirection.y, 2)));

        float yaw = std::atan2(lookDirection.x, lookDirection.z);

        EulerAngles newRotation;
        newRotation.Yaw = yaw;

        transform.SetRotation(newRotation);
    }
} // namespace GameLogic