#pragma once

#include <Frost.h>
#include <DirectXMath.h>

namespace GameLogic
{
    struct alignas(16) BillboardMaterialParameters
    {
        float BillboardSize;
        float Padding[3];
    };

    class Billboard : public Frost::Scripting::Script
    {
    public:
        void OnCreate() override;
        void OnUpdate(float deltaTime) override;

    private:
        BillboardMaterialParameters _params;
    };
} // namespace GameLogic