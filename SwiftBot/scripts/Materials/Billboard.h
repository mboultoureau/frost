#pragma once

#include <Frost.h>

namespace GameLogic
{

    class BillboardScript : public Frost::Scripting::Script
    {
    public:
        BillboardScript() = default;

        void OnUpdate(float deltaTime) override;

    private:
        void FaceCamera(const Frost::Math::Vector3& cameraPosition);
    };
} // namespace GameLogic