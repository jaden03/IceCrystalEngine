#pragma once

#include <Ice/Core/Component.h>
#include <Ice/Components/Physics/Collider.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>


#include <glm/glm.hpp>

#include <Ice/Core/PhysicsManager.h>

#include <functional>
#include <set>

class RigidBody : public Component
{
public:
    RigidBody(float mass = 1.0f, bool trigger = false);
    virtual ~RigidBody();

    void Ready() override;    // Called when actor/component is initialized
    void Update() override;   // Called every frame

    float mass;
    bool isTrigger = false;
    
    Collider* collider;

    //----------------------------------
    // Collision/Trigger Callbacks
    //----------------------------------
    // For non-trigger bodies colliding with other non-trigger bodies
    std::function<void(RigidBody*)> OnContactStarted;
    std::function<void(RigidBody*)> OnContacting;
    std::function<void(RigidBody*)> OnContactEnded;
    
    // For non-trigger bodies entering/exiting trigger bodies
    std::function<void(RigidBody*)> OnTriggerEntered;
    std::function<void(RigidBody*)> OnTriggerStay;
    std::function<void(RigidBody*)> OnTriggerExited;

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
    bool IsStatic() const {return isStatic;};

    //----------------------------------
    // Body Access
    //----------------------------------
    JPH::Body* GetBody() const { return body; }

    //----------------------------------
    // Internal - called by PhysicsManager
    //----------------------------------
    void FireContactStarted(RigidBody* other);
    void FireContacting(RigidBody* other);
    void FireContactEnded(RigidBody* other);
    void FireTriggerEntered(RigidBody* other);
    void FireTriggerStay(RigidBody* other);
    void FireTriggerExited(RigidBody* other);

private:
    JPH::Body* body = nullptr;
    bool isStatic = false;

    // Track active contacts for OnContacting
    std::set<RigidBody*> activeContacts;
    std::set<RigidBody*> activeTriggers;
};
