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

    //----------------------------------
    // Forces
    //----------------------------------
    void AddForce(glm::vec3 force);
    void AddTorque(glm::vec3 torque);
    void AddImpulse(glm::vec3 impulse);
    void AddAngularImpulse(glm::vec3 impulse);

    //------------------------------------
    // Velocities
    //------------------------------------
    void SetLinearVelocity(glm::vec3 velocity);
    glm::vec3 GetLinearVelocity();

    void SetAngularVelocity(glm::vec3 velocity);
    glm::vec3 GetAngularVelocity();

    //----------------------------------
    // Sleeping
    //----------------------------------
    bool IsActive();


    void SetKinematic(bool enabled);
    bool IsKinematic() const;

private:
    JPH::Body* body = nullptr;
};
