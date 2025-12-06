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

