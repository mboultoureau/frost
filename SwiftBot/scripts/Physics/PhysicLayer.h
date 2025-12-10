#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace GameLogic::ObjectLayers
{
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
    static constexpr JPH::ObjectLayer DEBRIS = 2;
    static constexpr JPH::ObjectLayer SENSOR = 3;
    static constexpr JPH::ObjectLayer CAMERA = 4;
    static constexpr JPH::ObjectLayer PLAYER = 5;
    static constexpr JPH::uint NUM_LAYERS = 6;
} // namespace GameLogic::ObjectLayers

namespace GameLogic::BroadPhaseLayers
{
    static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
    static constexpr JPH::BroadPhaseLayer MOVING(1);
    static constexpr JPH::BroadPhaseLayer DEBRIS(2);
    static constexpr JPH::BroadPhaseLayer SENSOR(3);
    static constexpr JPH::BroadPhaseLayer UNUSED(4);
    static constexpr JPH::uint NUM_LAYERS = 5;
} // namespace GameLogic::BroadPhaseLayers
