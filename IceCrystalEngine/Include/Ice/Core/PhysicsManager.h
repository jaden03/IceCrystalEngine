#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

// Simple stub layers (just to get things running)
class SimpleBroadPhaseLayer : public JPH::BroadPhaseLayerInterface
{
public:
    JPH::uint32 GetNumBroadPhaseLayers() const override { return 2; }
    JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
    {
        // Explicitly construct a BroadPhaseLayer
        return JPH::BroadPhaseLayer(static_cast<JPH::BroadPhaseLayer::Type>(layer));
    }
};

class SimpleObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override { return true; }
};

class SimpleObjectLayerPairFilter : public JPH::ObjectLayerPairFilter
{
public:
    bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override { return true; }
};

class PhysicsManager
{
public:
    static PhysicsManager& GetInstance()
    {
        static PhysicsManager instance; // Static local variable ensures a single instance
        return instance;
    }

    JPH::uint maxBodies = 1024;
    JPH::uint numBodyMutexes = 0;
    JPH::uint maxBodyPairs = 1024;
    JPH::uint maxContactConstraints = 1024;

    void Step(float fixedDeltaTime);

    // Minimal getter for the PhysicsSystem
    JPH::PhysicsSystem& GetSystem() { return physicsSystem; }

private:
    PhysicsManager();
    PhysicsManager(PhysicsManager const&) = delete;
    void operator=(PhysicsManager const&) = delete;

    // Jolt essentials
    JPH::TempAllocatorImpl tempAllocator{ 64 * 1024 * 1024 }; // 64 MB
    JPH::JobSystemThreadPool jobSystem{ JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers };
    JPH::PhysicsSystem physicsSystem;

    SimpleBroadPhaseLayer broadPhase;
    SimpleObjectVsBroadPhaseLayerFilter layerFilter;
    SimpleObjectLayerPairFilter pairFilter;
};
