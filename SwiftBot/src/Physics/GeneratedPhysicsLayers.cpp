#include "GeneratedPhysicsLayers.h"
#include <cassert>

GameBroadPhaseLayerInterface::GameBroadPhaseLayerInterface()
{
    mObjectToBroadPhase[ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[ObjectLayers::MOVING] = BroadPhaseLayers::MOVING;
    mObjectToBroadPhase[ObjectLayers::DEBRIS] = BroadPhaseLayers::DEBRIS;
    mObjectToBroadPhase[ObjectLayers::SENSOR] = BroadPhaseLayers::SENSOR;
}

JPH::uint
GameBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer
GameBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
    // JPH_ASSERT(inLayer < ObjectLayers::NUM_LAYERS);
    return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char*
GameBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
{
    switch ((JPH::BroadPhaseLayer::Type)inLayer)
    {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
            return "MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DEBRIS:
            return "DEBRIS";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::SENSOR:
            return "SENSOR";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::UNUSED:
            return "UNUSED";
        default:
            return "Unknown";
    }
}
#endif

bool
GameObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
    switch (inObject1)
    {
        case ObjectLayers::NON_MOVING:
            return inObject2 == ObjectLayers::MOVING || inObject2 == ObjectLayers::DEBRIS;
        case ObjectLayers::MOVING:
            return inObject2 == ObjectLayers::NON_MOVING || inObject2 == ObjectLayers::MOVING ||
                   inObject2 == ObjectLayers::DEBRIS;
        case ObjectLayers::DEBRIS:
            return inObject2 == ObjectLayers::NON_MOVING || inObject2 == ObjectLayers::MOVING;
        case ObjectLayers::SENSOR:
            return false;
        default:
            return false;
    }
}

bool
GameObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
    switch (inLayer1)
    {
        case ObjectLayers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::DEBRIS;
        case ObjectLayers::MOVING:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING ||
                   inLayer2 == BroadPhaseLayers::DEBRIS;
        case ObjectLayers::DEBRIS:
            return inLayer2 == BroadPhaseLayers::NON_MOVING || inLayer2 == BroadPhaseLayers::MOVING;
        case ObjectLayers::SENSOR:
            return false;
        default:
            return false;
    }
}
