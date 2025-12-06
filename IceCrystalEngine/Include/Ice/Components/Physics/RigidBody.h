#pragma once

#include <Ice/Core/Component.h>
#include <Ice/Components/Physics/Collider.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>


#include <glm/glm.hpp>

#include <Ice/Core/PhysicsManager.h>

class RigidBody : public Component
{
public:
    RigidBody(float mass = 1.0f);
    virtual ~RigidBody();

    void Ready() override;    // Called when actor/component is initialized
    void Update() override;   // Called every frame

    float mass;
    
    Collider* collider;
    JPH::Body* body = nullptr;

private:

};
