#include <Ice/Managers/PhysicsManager.h>
#include <Ice/Components/Physics/RigidBody.h>

PhysicsManager& PhysicsManager::GetInstance()
{
    static PhysicsManager instance; // Static local variable ensures a single instance
    return instance;
}

void PhysicsContactListener::OnContactAdded(
    const JPH::Body& inBody1,
    const JPH::Body& inBody2,
    const JPH::ContactManifold& inManifold,
    JPH::ContactSettings& ioSettings)
{
    RigidBody* rb1 = reinterpret_cast<RigidBody*>(inBody1.GetUserData());
    RigidBody* rb2 = reinterpret_cast<RigidBody*>(inBody2.GetUserData());
    
    if (!rb1 || !rb2) return;
    
    bool isTrigger1 = inBody1.IsSensor();
    bool isTrigger2 = inBody2.IsSensor();
    
    // Store contact pair
    BodyPairKey key(inBody1.GetID(), inBody2.GetID());
    activeContacts[key] = {rb1, rb2};
    
    if (isTrigger1 || isTrigger2)
    {
        // Trigger interaction
        if (!isTrigger1 && isTrigger2)
        {
            rb1->FireTriggerEntered(rb2);
        }
        if (!isTrigger2 && isTrigger1)
        {
            rb2->FireTriggerEntered(rb1);
        }
    }
    else
    {
        // Normal collision
        rb1->FireContactStarted(rb2);
        rb2->FireContactStarted(rb1);
    }
}

void PhysicsContactListener::OnContactPersisted(
    const JPH::Body& inBody1,
    const JPH::Body& inBody2,
    const JPH::ContactManifold& inManifold,
    JPH::ContactSettings& ioSettings)
{
    // OnContacting and OnTriggerStay are handled in RigidBody::Update()
}

void PhysicsContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
    BodyPairKey key(inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID());
    
    auto it = activeContacts.find(key);
    if (it == activeContacts.end())
    {
        return;
    }
    
    RigidBody* rb1 = it->second.first;
    RigidBody* rb2 = it->second.second;
    
    if (rb1 && rb2)
    {
        JPH::Body* body1 = rb1->GetBody();
        JPH::Body* body2 = rb2->GetBody();
        
        if (!body1 || !body2)
        {
            activeContacts.erase(it);
            return;
        }
        
        bool isTrigger1 = body1->IsSensor();
        bool isTrigger2 = body2->IsSensor();
        
        
        if (isTrigger1 || isTrigger2)
        {
            if (!isTrigger1 && isTrigger2)
            {
                rb1->FireTriggerExited(rb2);
            }
            if (!isTrigger2 && isTrigger1)
            {
                rb2->FireTriggerExited(rb1);
            }
        }
        else
        {
            rb1->FireContactEnded(rb2);
            rb2->FireContactEnded(rb1);
        }
    }
    
    activeContacts.erase(it);
}

PhysicsManager::PhysicsManager()
{
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    
    physicsSystem.Init(
        maxBodies,
        numBodyMutexes,
        maxBodyPairs,
        maxContactConstraints,
        broadPhase,
        layerFilter,
        pairFilter
    );

    // Register contact listener
    physicsSystem.SetContactListener(&contactListener);

    initialized = true;
}

PhysicsManager::~PhysicsManager()
{
    initialized = false;
    delete JPH::Factory::sInstance;
}


void PhysicsManager::Step(float fixedDeltaTime)
{
    physicsSystem.Update(
        fixedDeltaTime,
        1,                // collision steps
        &tempAllocator,   // <-- REQUIRED
        &jobSystem        // <-- REQUIRED
    );
}


void PhysicsManager::SetGravity(const glm::vec3 gravity)
{
    physicsSystem.SetGravity(ToJolt(gravity));
    
    // Force a verification in release too
    volatile auto g = physicsSystem.GetGravity();
    (void)g;
}