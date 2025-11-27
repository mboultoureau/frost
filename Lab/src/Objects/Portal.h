#pragma once

#include "Frost.h"
#include <memory>

using namespace Frost;
using namespace Frost::Component;
using namespace Frost::Math;

class Portal
{
public:
    Portal(Vector3 position, EulerAngles rotation);
    void LinkTo(Portal* other);
    void Update();

    std::shared_ptr<Texture> GetRenderTarget() const { return _renderTarget; }
    Transform& GetTransform() { return _portalObject.GetComponent<Transform>(); }

private:
    GameObject _portalObject;
    GameObject _frameObject;
    GameObject _cameraObject;
    std::shared_ptr<Texture> _renderTarget;

    Portal* _linkedPortal{ nullptr };
};
