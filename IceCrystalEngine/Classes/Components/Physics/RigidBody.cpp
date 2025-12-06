#include <Ice/Components/Physics/RigidBody.h>

RigidBody::RigidBody(float mass) : mass(mass)
{
}

void RigidBody::Ready()
{
    // Ensure component is attached to an actor
    if (!owner) return;
    // Ensure there is a collider on the actor
    collider = owner->GetComponent<Collider>();
    if (!collider) return;

    // Make sure the collider actually has a shape
    auto shape = collider->GetShape();
    if (!shape) return;

    glm::vec3 pos = transform->position;
    
    JPH::EMotionType motionType = mass > 0.0f ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static;
    // Create a box body in Jolt
    JPH::BodyCreationSettings settings(
        shape,
        JPH::Vec3(pos.x, pos.y, pos.z),
        JPH::Quat::sIdentity(),
        motionType,
        0 // object layer
    );
    body = PhysicsManager::GetInstance().GetSystem().GetBodyInterface().CreateBody(settings);

    JPH::EActivation activation = (motionType == JPH::EMotionType::Dynamic) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().AddBody(body->GetID(), activation);
}

void RigidBody::Update()
{
    if (!body || !owner) return;

    // Sync physics position back to actor transform
    JPH::Vec3 pos;
    JPH::Quat rot;
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().GetPositionAndRotation(body->GetID(), pos, rot);

    owner->transform->position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
    owner->transform->SetRotation(glm::quat(rot.GetW(), -rot.GetX(), rot.GetY(), -rot.GetZ()));
}

RigidBody::~RigidBody() {
    if (body) {
        PhysicsManager::GetInstance().GetSystem().GetBodyInterface().RemoveBody(body->GetID());
        body = nullptr;
    }
}