#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

class ShapeCouldNotBeRegistered {};
class ShapeRegistry {
public:
    using ShapeID = uint32_t;

    ShapeID registerShape(JPH::ShapeRefC shapeRef) {
        ShapeID id = next_id++;
        shapeRefs.emplace(id, shapeRef);
        return id;
    }

    JPH::ShapeRefC& getRef(ShapeID id) {
        return shapeRefs.at(id);
    }

    void unregisterShapeRefC(ShapeID id) {
        shapeRefs.erase(id);
    }

private:
    ShapeID next_id = 1;
    std::unordered_map < ShapeID, JPH::ShapeRefC > shapeRefs;
};