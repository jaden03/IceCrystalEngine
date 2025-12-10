#include <Ice/Components/Physics/RigidBody.h>
#include <Ice/Editor/WebEditorManager.h>
#include <Ice/Editor/EditorUI.h>

RigidBody::RigidBody(float mass, bool trigger) : mass(mass), isTrigger(trigger) {}

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

    isStatic = mass <= 0.0f;
    JPH::EMotionType motionType = isStatic ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic;
    // Create a box body in Jolt
    JPH::BodyCreationSettings settings(
        shape,
        JPH::Vec3(pos.x, pos.y, pos.z),
        JPH::Quat(ToJolt(transform->rotation)),
        motionType,
        0 // object layer
    );
    settings.mIsSensor = isTrigger;

    if (!isStatic) {
        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = mass;
    }
    
    body = PhysicsManager::GetInstance().GetSystem().GetBodyInterface().CreateBody(settings);

    // Store pointer to this RigidBody in the body's user data
    body->SetUserData(reinterpret_cast<JPH::uint64>(this));

    JPH::EActivation activation = (motionType == JPH::EMotionType::Dynamic) ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().AddBody(body->GetID(), activation);
}

void RigidBody::Update()
{
    if (!body || !owner || isStatic) return;

    // Don't sync physics to transform when engine is paused (allows manual editing)
    bool isEnginePaused = EditorUI::GetInstance().IsEnginePaused() || WebEditorManager::GetInstance().IsEnginePaused();
    if (isEnginePaused)
    {
        // Sync transform changes TO the physics body instead (allows manual editing)
        JPH::Vec3 currentPos;
        JPH::Quat currentRot;
        PhysicsManager::GetInstance().GetSystem().GetBodyInterface().GetPositionAndRotation(body->GetID(), currentPos, currentRot);
        
        glm::vec3 transformPos = owner->transform->position;
        glm::quat transformRot = owner->transform->rotation;
        
        JPH::Vec3 newPos(transformPos.x, transformPos.y, transformPos.z);
        JPH::Quat newRot(transformRot.x, transformRot.y, transformRot.z, transformRot.w);
        
        // Check if position or rotation changed externally (from editor/gizmo)
        bool posChanged = glm::distance(glm::vec3(currentPos.GetX(), currentPos.GetY(), currentPos.GetZ()), transformPos) > 0.001f;
        bool rotChanged = glm::abs(currentRot.GetX() - transformRot.x) > 0.001f ||
                          glm::abs(currentRot.GetY() - transformRot.y) > 0.001f ||
                          glm::abs(currentRot.GetZ() - transformRot.z) > 0.001f ||
                          glm::abs(currentRot.GetW() - transformRot.w) > 0.001f;
        
        if (posChanged || rotChanged)
        {
            // Update physics body with new transform
            PhysicsManager::GetInstance().GetSystem().GetBodyInterface().SetPositionAndRotation(
                body->GetID(), newPos, newRot, JPH::EActivation::DontActivate);
            
            // Clear velocities to prevent object from continuing previous motion when unpaused
            body->SetLinearVelocity(JPH::Vec3::sZero());
            body->SetAngularVelocity(JPH::Vec3::sZero());
        }
        
        return;
    }

    // Sync physics position back to actor transform (normal runtime behavior)
    JPH::Vec3 pos;
    JPH::Quat rot;
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().GetPositionAndRotation(body->GetID(), pos, rot);

    owner->transform->position = glm::vec3(pos.GetX(), pos.GetY(), pos.GetZ());
    owner->transform->rotation = glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());

    // Fire OnContacting for all active contacts
    for (RigidBody* other : activeContacts)
    {
        if (OnContacting)
            OnContacting(other);
    }
    
    // Fire OnTriggerStay for all active triggers
    for (RigidBody* other : activeTriggers)
    {
        if (OnTriggerStay)
            OnTriggerStay(other);
    }
}

RigidBody::~RigidBody() {
    if (body) {
        // Check if PhysicsManager still exists and is valid
        PhysicsManager& physicsManager = PhysicsManager::GetInstance();
        
        // Only remove body if physics system is still running
        // During shutdown, Jolt cleans up all bodies automatically
        if (physicsManager.IsInitialized()) {
            JPH::BodyInterface& bodyInterface = physicsManager.GetSystem().GetBodyInterface();
            JPH::BodyID bodyID = body->GetID();
            
            bodyInterface.RemoveBody(bodyID);
            bodyInterface.DestroyBody(bodyID);
        }
        
        body = nullptr;
    }
}

void RigidBody::FireContactStarted(RigidBody* other)
{
    activeContacts.insert(other);
    if (OnContactStarted)
        OnContactStarted(other);
}

void RigidBody::FireContacting(RigidBody* other)
{
    // Already handled in Update() via activeContacts
}

void RigidBody::FireContactEnded(RigidBody* other)
{
    activeContacts.erase(other);
    if (OnContactEnded)
        OnContactEnded(other);
}

void RigidBody::FireTriggerEntered(RigidBody* other)
{
    activeTriggers.insert(other);
    if (OnTriggerEntered)
        OnTriggerEntered(other);
}

void RigidBody::FireTriggerStay(RigidBody* other)
{
    // Already handled in Update() via activeTriggers
}

void RigidBody::FireTriggerExited(RigidBody* other)
{
    activeTriggers.erase(other);
    if (OnTriggerExited)
        OnTriggerExited(other);
}



void RigidBody::AddForce(glm::vec3 force)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    body->AddForce(ToJolt(force));
}
void RigidBody::AddTorque(glm::vec3 torque)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    body->AddTorque(ToJolt(torque));
}
void RigidBody::AddImpulse(glm::vec3 impulse)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    body->AddImpulse(ToJolt(impulse));
}
void RigidBody::AddAngularImpulse(glm::vec3 impulse)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    body->AddAngularImpulse(ToJolt(impulse));
}

void RigidBody::SetLinearVelocity(glm::vec3 velocity)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    body->SetLinearVelocity(ToJolt(velocity));
}
glm::vec3 RigidBody::GetLinearVelocity()
{
    return ToGLM(body->GetLinearVelocity());
}

void RigidBody::SetAngularVelocity(glm::vec3 velocity)
{
    PhysicsManager::GetInstance().GetSystem().GetBodyInterface().ActivateBody(body->GetID());
    
    body->SetAngularVelocity(ToJolt(velocity));
}
glm::vec3 RigidBody::GetAngularVelocity()
{
    return ToGLM(body->GetAngularVelocity());
}


bool RigidBody::IsActive()
{
    return body->IsActive();
}

void RigidBody::SetKinematic(bool enabled)
{
    auto& iface = PhysicsManager::GetInstance().GetSystem().GetBodyInterface();

    if (enabled)
    {
        iface.SetMotionType(body->GetID(), JPH::EMotionType::Kinematic, JPH::EActivation::DontActivate);

        // Kinematic bodies should not keep any dynamic velocity
        body->SetLinearVelocity(JPH::Vec3::sZero());
        body->SetAngularVelocity(JPH::Vec3::sZero());
        return;
    }

    if (mass == 0.0f)
    {
        iface.SetMotionType(body->GetID(), JPH::EMotionType::Static, JPH::EActivation::DontActivate);

        // Just in case, clear velocity (static bodies shouldn't move)
        body->SetLinearVelocity(JPH::Vec3::sZero());
        body->SetAngularVelocity(JPH::Vec3::sZero());
    }
    else
    {
        JPH::MassProperties mp = body->GetShape()->GetMassProperties();
        
        float scale = mass / mp.mMass;
        mp.mMass = mass;
        mp.mInertia *= scale;

        body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::All, mp);

        iface.SetMotionType(body->GetID(), JPH::EMotionType::Dynamic, JPH::EActivation::Activate);
    }
}
bool RigidBody::IsKinematic() const
{
    auto& iface = PhysicsManager::GetInstance().GetSystem().GetBodyInterface();
    return iface.GetMotionType(body->GetID()) == JPH::EMotionType::Kinematic;
}




