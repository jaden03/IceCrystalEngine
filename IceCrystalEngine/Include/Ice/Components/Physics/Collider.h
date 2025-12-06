#pragma once

#include <Ice/Core/Component.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

class Collider : public Component
{
public:
    virtual ~Collider() = default;

    virtual JPH::ShapeRefC GetShape() const = 0;
};
