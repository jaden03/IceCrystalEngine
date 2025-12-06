#pragma once

#include <Ice/Components/Physics/Collider.h>
#include "Jolt/Physics/Collision/Shape/BoxShape.h"

class BoxCollider : public Collider
{
public:
    BoxCollider(const glm::vec3 &size) : size(size)
    {
        JPH::BoxShapeSettings settings(JPH::Vec3(size.x, size.y, size.z));
        settings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult result = settings.Create();
        shape = result.Get(); // now shape is valid
    }

    glm::vec3 size;
    JPH::ShapeRefC GetShape() const override { return shape; }

private:
    JPH::ShapeRefC shape;
};
