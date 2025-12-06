#pragma once

#include <Ice/Components/Physics/Collider.h>
#include "Jolt/Physics/Collision/Shape/BoxShape.h"

class BoxCollider : public Collider
{
public:
    BoxCollider(const glm::vec3 &halfExtents) : halfExtents(halfExtents)
    {
        JPH::BoxShapeSettings settings(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
        settings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult result = settings.Create();
        shape = result.Get(); // now shape is valid
    }

    JPH::ShapeRefC GetShape() const override { return shape; }

private:
    glm::vec3 halfExtents;
    JPH::ShapeRefC shape;
};
