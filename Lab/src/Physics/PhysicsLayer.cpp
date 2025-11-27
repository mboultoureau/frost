#include "PhysicsLayer.h"

GameBroadPhaseLayerInterface::GameBroadPhaseLayerInterface()
{
    mObjectToBroadPhase[ObjectLayers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
    mObjectToBroadPhase[ObjectLayers::PLAYER] = BroadPhaseLayers::MOVING;
    mObjectToBroadPhase[ObjectLayers::BULLET] = BroadPhaseLayers::MOVING;
    mObjectToBroadPhase[ObjectLayers::CARGO] = BroadPhaseLayers::MOVING;
    mObjectToBroadPhase[ObjectLayers::GOAL] = BroadPhaseLayers::NON_MOVING;
}

JPH::uint
GameBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const
{
    return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer
GameBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
    FT_ENGINE_ASSERT(inLayer < ObjectLayers::NUM_LAYERS, "Invalid object layer");
    return mObjectToBroadPhase[inLayer];
}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
const char*
GameBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
{
    switch (static_cast<JPH::BroadPhaseLayer::Type>(inLayer))
    {
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
            return "NON_MOVING";
        case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
            return "MOVING";
        default:
            FT_ENGINE_ASSERT(false, "Invalid broad phase layer");
            return "UNKNOWN";
    }
}
#endif

bool
GameObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
    switch (inObject1)
    {
        case ObjectLayers::NON_MOVING:
            return inObject2 != ObjectLayers::NON_MOVING;
        case ObjectLayers::PLAYER:
            return inObject2 == ObjectLayers::NON_MOVING;
        case ObjectLayers::BULLET:
            return inObject2 == ObjectLayers::CARGO || inObject2 == ObjectLayers::NON_MOVING;
        case ObjectLayers::CARGO:
            return inObject2 != ObjectLayers::PLAYER;
        case ObjectLayers::GOAL:
            return inObject2 == ObjectLayers::CARGO;
        default:
            FT_ENGINE_ASSERT(false, "Invalid object layer");
            return false;
    }
}

bool
GameObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
    switch (inLayer1)
    {
        case ObjectLayers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case ObjectLayers::PLAYER:
            return true;
        case ObjectLayers::BULLET:
            return true;
        case ObjectLayers::CARGO:
            return true;
        case ObjectLayers::GOAL:
            return inLayer2 == BroadPhaseLayers::MOVING;
        default:
            FT_ENGINE_ASSERT(false, "Invalid object layer");
            return false;
    }
}
