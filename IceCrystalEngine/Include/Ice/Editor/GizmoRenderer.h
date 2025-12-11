#pragma once

#include <glm/glm.hpp>

// Forward declarations
class Actor;
class Camera;

// Gizmo operation modes
enum class GizmoMode
{
    Translate,
    Rotate,
    Scale
};

// Gizmo axis selection
enum class GizmoAxis
{
    None,
    X,
    Y,
    Z,
    XY,
    XZ,
    YZ
};

// Stub header for GizmoRenderer - actual implementation is in IceCrystalEngineEditor project
class GizmoRenderer
{
public:
    static GizmoRenderer& GetInstance();
    
    // Initialization
    void Initialize();
    
    // Mode management
    void SetMode(GizmoMode mode);
    GizmoMode GetMode() const;
    
    // Enable/disable
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    
    // Interaction state
    bool IsDragging() const;
    
    // Mouse interaction
    GizmoAxis GetHoveredAxis(const glm::vec2& mousePos, const glm::vec2& screenSize,
                             const glm::mat4& view, const glm::mat4& projection,
                             const glm::vec3& gizmoPosition);
    
    void HandleMouseDown(const glm::vec2& mousePos, const glm::vec2& screenSize,
                        const glm::mat4& view, const glm::mat4& projection, Actor* actor);
    
    void HandleMouseMove(const glm::vec2& mousePos, const glm::vec2& screenSize,
                        const glm::mat4& view, const glm::mat4& projection);
    
    void HandleMouseUp();
    
    // Rendering
    void RenderGizmos(Actor* actor, Camera* camera);
};