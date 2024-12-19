#pragma once
#ifndef PHYSICS_MANAGER_H
#define PHYSICS_MANAGER_H

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <glm/glm.hpp>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>


class PhysicsManager
{
public:

    static PhysicsManager& GetInstance()
    {
        static PhysicsManager instance;
        return instance;
    }

    void Update(float deltaTime);

    glm::vec3 gravity = glm::vec3(0.0f, -9.81f, 0.0f);

private:

    // Physics system and the memory/job system objects
    JPH::PhysicsSystem physicsSystem;
    JPH::TempAllocator* tempAllocator;
    JPH::JobSystem* jobSystem;

    PhysicsManager();
    ~PhysicsManager();

    PhysicsManager(PhysicsManager const&) = delete;
    PhysicsManager& operator=(PhysicsManager const&) = delete;
    
};

#endif
