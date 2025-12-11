#pragma once

#include "Frost.h"
#include "Player/Player.h"
#include <Frost/Scene/Components/Script.h>
#include <string>

using namespace Frost;
using namespace Frost::Math;
using namespace Frost::Component;

enum PortalType
{
    Entry,
    Exit,
    BothWays
};

class Portal;
class PortalScript : public Script
{
public:
    PortalScript(GameObject playerId, PortalType type, Portal* linkedPortal, Portal* parent);
    void OnInitialize() override;
    float CalculatePortalFOV(const Math::Vector3& cameraPos,
                             const Math::Vector3& portalPos,
                             const Math::Vector3& portalScale,
                             bool isVertical);
    void OnUpdate(float deltaTime) override;
    void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;
    void WarpPlayer();

    GameObject playerId;

    PortalType portalType;

    Portal* linkedPortal;
    Portal* parentPortal;

    const std::chrono::milliseconds _chromaticAberrationDuration = 2000ms;
    const float _chromaticAberrationFactor = 0.0f;
    Timer _chromaticAberrationTimer;
    bool _inChromaticEffect = false;

private:
    void UpdateChromaticAberrationEffect();
    void UpdateLinkedCameraRef();
    void UpdateCameraRenderTarget();
    void UpdateCameraPos();
    bool _materialLinkPending = false;
};

class Portal
{
public:
    Portal(Vector3 position, EulerAngles rotation, Vector3 scale, Player* player);
    void SetupPortal(PortalType type, Portal* otherPortal);
    GameObject _portal;
    GameObject _frameObject;
    GameObject _cameraObject;
    std::shared_ptr<Texture> _renderTarget;

private:
    Player* _player;
};