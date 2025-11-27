#pragma once

#include <Jolt/Core/Core.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include "Frost.h"

namespace ObjectLayers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer PLAYER = 1;
    static constexpr JPH::ObjectLayer BULLET = 2;
    static constexpr JPH::ObjectLayer CARGO = 3;
    static constexpr JPH::ObjectLayer CHECKPOINT = 4;
    static constexpr JPH::ObjectLayer NO_COLLIDE = 5;
    static constexpr JPH::ObjectLayer CAMERA = 6;
    static constexpr JPH::ObjectLayer PORTAL = 7;
    static constexpr JPH::ObjectLayer WATER = 8;
    static constexpr JPH::ObjectLayer BOOST = 9;
    static constexpr JPH::ObjectLayer NUM_LAYERS = 10;
} // namespace ObjectLayers

namespace BroadPhaseLayers
{

    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::uint NUM_LAYERS(2);
} // namespace BroadPhaseLayers

class GameBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
{
public:
    GameBroadPhaseLayerInterface();
    JPH::uint GetNumBroadPhaseLayers() const override;
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif

private:
    JPH::BroadPhaseLayer mObjectToBroadPhase[ObjectLayers::NUM_LAYERS];
};

class GameObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
};

class GameObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
};