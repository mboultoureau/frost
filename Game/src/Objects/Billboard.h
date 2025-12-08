#pragma once

#include "Frost.h"
#include "Frost/Scene/Components/Script.h"
#include <Frost/Utils/Math/Vector.h>
#include <Frost/Asset/Texture.h>
#include <DirectXMath.h>

using namespace Frost;
using namespace Frost::Math;

struct alignas(16) BillboardMaterialParameters
{

    DirectX::XMVECTOR CameraPosition;

    float BillboardSize;

    float padding[3];
};

class Billboard
{
public:
    Billboard(Vector3 pos, float size, const std::string& texturePath);

    void _SetClosestPlayerPosToShader();

    GameObject GetGameObject() const { return _billboardObject; }

    void UpdateShader(float dt);

    void DestroyObject();

private:
    GameObject _billboardObject;

    BillboardMaterialParameters _params;

    std::shared_ptr<Texture> _texture;
};

class BillboardScript : public Script
{
    Billboard* _billboard;

public:
    BillboardScript(Billboard* billboard) : _billboard{ billboard } {};
    void OnUpdate(float deltaTime) override;
};