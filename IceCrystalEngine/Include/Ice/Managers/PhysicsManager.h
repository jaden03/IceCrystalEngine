#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <map>
class RigidBody;



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



// Helpers
// --------------------
// glm::vec3 → JPH::Vec3
// --------------------
inline JPH::Vec3 ToJolt(const glm::vec3& v)
{
    return JPH::Vec3(v.x, v.y, v.z);
}
// --------------------
// JPH::Vec3 → glm::vec3
// --------------------
inline glm::vec3 ToGLM(const JPH::Vec3& v)
{
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}
// glm::quat → JPH::Quat
inline JPH::Quat ToJolt(const glm::quat& q)
{
    // glm is (w, x, y, z)
    // Jolt is also (w, x, y, z)
    return JPH::Quat(q.x, q.y, q.z, q.w);
}
// JPH::Quat → glm::quat
inline glm::quat ToGLM(const JPH::Quat& q)
{
    // glm::quat constructor is (w, x, y, z)
    return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

struct BodyPairKey
{
    JPH::BodyID body1;
    JPH::BodyID body2;
    
    BodyPairKey(JPH::BodyID b1, JPH::BodyID b2)
    {
        // Always store smaller ID first for consistent lookups
        if (b1.GetIndexAndSequenceNumber() < b2.GetIndexAndSequenceNumber())
        {
            body1 = b1;
            body2 = b2;
        }
        else
        {
            body1 = b2;
            body2 = b1;
        }
    }
    
    bool operator<(const BodyPairKey& other) const
    {
        if (body1.GetIndexAndSequenceNumber() != other.body1.GetIndexAndSequenceNumber())
            return body1.GetIndexAndSequenceNumber() < other.body1.GetIndexAndSequenceNumber();
        return body2.GetIndexAndSequenceNumber() < other.body2.GetIndexAndSequenceNumber();
    }
};

class PhysicsContactListener : public JPH::ContactListener
{
public:
    virtual JPH::ValidateResult OnContactValidate(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        JPH::RVec3Arg inBaseOffset,
        const JPH::CollideShapeResult& inCollisionResult) override
    {
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold,
        JPH::ContactSettings& ioSettings) override;

    virtual void OnContactPersisted(
        const JPH::Body& inBody1,
        const JPH::Body& inBody2,
        const JPH::ContactManifold& inManifold,
        JPH::ContactSettings& ioSettings) override;

    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

private:
    std::map<BodyPairKey, std::pair<RigidBody*, RigidBody*>> activeContacts;
};


class PhysicsManager
{
public:
    static PhysicsManager& GetInstance();

    JPH::uint maxBodies = 1024;
    JPH::uint numBodyMutexes = 0;
    JPH::uint maxBodyPairs = 1024;
    JPH::uint maxContactConstraints = 1024;

    void Step(float fixedDeltaTime);

    // Minimal getter for the PhysicsSystem
    JPH::PhysicsSystem& GetSystem() { return physicsSystem; }

    bool IsInitialized() const { return initialized; }

    void SetGravity(glm::vec3 gravity);

private:
    bool initialized = false;
    
    PhysicsManager();
    PhysicsManager(PhysicsManager const&) = delete;
    void operator=(PhysicsManager const&) = delete;
    ~PhysicsManager();

    // Jolt essentials
    JPH::TempAllocatorImpl tempAllocator{ 64 * 1024 * 1024 }; // 64 MB
    JPH::JobSystemThreadPool jobSystem{ JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers };
    JPH::PhysicsSystem physicsSystem;

    SimpleBroadPhaseLayer broadPhase;
    SimpleObjectVsBroadPhaseLayerFilter layerFilter;
    SimpleObjectLayerPairFilter pairFilter;

    PhysicsContactListener contactListener;
};
