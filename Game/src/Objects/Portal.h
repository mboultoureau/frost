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

class PortalScript : public Script
{
public:
    PortalScript(GameObject playerId, PortalType type, GameObject& linkedId);
    PortalScript(GameObject playerId, PortalType type);
    void OnInitialize() override;
    void OnUpdate(float deltaTime) override;
    void OnCollisionEnter(BodyOnContactParameters params, float deltaTime) override;
    void WarpPlayer();

    GameObject playerId;

    PortalType portalType;

    std::optional<GameObject> linkedPortalId;

    const std::chrono::milliseconds _chromaticAberrationDuration = 2000ms;
    const float _chromaticAberrationFactor = 0.1f;
    Timer _chromaticAberrationTimer;
    bool _inChromaticEffect = false;
};

class Portal
{
public:
    Portal(Vector3 position, EulerAngles rotation, Vector3 scale, Player* player);
    void SetupPortal(PortalType type, GameObject& otherPortal);
    void SetupPortal(PortalType type);
    GameObject _portal;

private:
    Player* _player;
};