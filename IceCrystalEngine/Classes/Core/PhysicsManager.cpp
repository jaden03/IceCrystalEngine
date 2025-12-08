#include <Ice/Core/PhysicsManager.h>

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

