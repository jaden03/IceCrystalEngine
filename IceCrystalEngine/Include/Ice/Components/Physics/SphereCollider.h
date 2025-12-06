#pragma once

#include <Ice/Components/Physics/Collider.h>
#include "Jolt/Physics/Collision/Shape/SphereShape.h"

class SphereCollider : public Collider
{
public:
    SphereCollider(const float &radius) : radius(radius)
    {
        JPH::SphereShapeSettings settings(radius);
        settings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult result = settings.Create();
        shape = result.Get(); // now shape is valid
    }

    float radius;
    JPH::ShapeRefC GetShape() const override { return shape; }

private:
    JPH::ShapeRefC shape;
};
