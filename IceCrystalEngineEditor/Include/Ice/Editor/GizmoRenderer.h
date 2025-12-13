#pragma once

#ifndef GIZMO_RENDERER_H
#define GIZMO_RENDERER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <vector>

class Actor;
class Camera;

enum class GizmoMode
{
    Translate,
    Rotate,
    Scale
};

enum class GizmoAxis
{
    None,
    X,
    Y,
    Z
};

class GizmoRenderer
{
public:
    static GizmoRenderer& GetInstance();
    
    void Initialize();
    void Cleanup();

    // Render gizmos for the selected actor
    void RenderGizmos(Actor* selectedActor, Camera* camera);

    // Set the current gizmo mode
    void SetMode(GizmoMode mode);
    GizmoMode GetMode() const;

    // Enable/disable gizmo rendering
    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    // Check if mouse is hovering over a gizmo axis
    GizmoAxis HitTest(const glm::vec2& mousePos, const glm::vec2& screenSize, 
                      const glm::mat4& view, const glm::mat4& projection, 
                      const glm::vec3& gizmoPosition);

    // Handle mouse drag for manipulating transforms
    void StartDrag(GizmoAxis axis, const glm::vec2& mousePos, 
                   const glm::vec2& screenSize, const glm::mat4& view, 
                   const glm::mat4& projection, Actor* actor);
    
    void UpdateDrag(const glm::vec2& mousePos, const glm::vec2& screenSize,
                    const glm::mat4& view, const glm::mat4& projection);
    
    void EndDrag();

    bool IsDragging() const;

    // High-level mouse interaction methods
    void HandleMouseDown(const glm::vec2& mousePos, const glm::vec2& screenSize,
                        const glm::mat4& view, const glm::mat4& projection, Actor* actor);
    
    void HandleMouseMove(const glm::vec2& mousePos, const glm::vec2& screenSize,
                        const glm::mat4& view, const glm::mat4& projection);
    
    void HandleMouseUp();

    // Get currently hovered axis (for highlighting)
    GizmoAxis GetHoveredAxis(const glm::vec2& mousePos, const glm::vec2& screenSize,
                            const glm::mat4& view, const glm::mat4& projection,
                            const glm::vec3& gizmoPosition);

private:
    GizmoRenderer();
    ~GizmoRenderer();
    GizmoRenderer(GizmoRenderer const&) = delete;
    void operator=(GizmoRenderer const&) = delete;

    // OpenGL resources
    GLuint lineShaderProgram;
    GLuint arrowVAO, arrowVBO;
    GLuint circleVAO, circleVBO;
    GLuint cubeVAO, cubeVBO;

    // State
    GizmoMode currentMode;
    bool enabled;
    bool isDragging;
    GizmoAxis dragAxis;
    GizmoAxis hoveredAxis;
    Actor* dragActor;
    glm::vec3 dragStartWorldPos;
    glm::vec3 dragStartActorPos;
    glm::vec3 dragStartActorRotation;
    glm::vec3 dragStartActorScale;
    glm::vec2 dragStartMousePos;

    // Rendering functions
    void RenderTranslateGizmo(const glm::vec3& position, const glm::mat4& view, 
                              const glm::mat4& projection);
    
    void RenderRotateGizmo(const glm::vec3& position, const glm::mat4& view, 
                           const glm::mat4& projection);
    
    void RenderScaleGizmo(const glm::vec3& position, const glm::mat4& view, 
                          const glm::mat4& projection);

    // Draw primitives
    void DrawArrow(const glm::vec3& start, const glm::vec3& direction, 
                   float length, const glm::vec3& color, const glm::mat4& view, 
                   const glm::mat4& projection);
    
    void DrawLine(const glm::vec3& start, const glm::vec3& end, 
                  const glm::vec3& color, const glm::mat4& view, 
                  const glm::mat4& projection);
    
    void DrawCircle(const glm::vec3& center, const glm::vec3& normal, 
                    float radius, const glm::vec3& color, const glm::mat4& view, 
                    const glm::mat4& projection);

    // Utility functions
    void InitializeShaders();
    void InitializeGeometry();
    glm::vec3 ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize,
                            const glm::mat4& view, const glm::mat4& projection,
                            float depth);
    
    float CalculateGizmoSize(const glm::vec3& position, const glm::mat4& view, 
                             const glm::mat4& projection, const glm::vec2& screenSize);

    // Ray-casting for gizmo hit detection
    bool RayIntersectsArrow(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                           const glm::vec3& arrowStart, const glm::vec3& arrowDir,
                           float arrowLength, float threshold);
    
    bool RayIntersectsCircle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                            const glm::vec3& circleCenter, const glm::vec3& circleNormal,
                            float radius, float threshold);

    glm::vec3 ProjectPointOntoPlane(const glm::vec3& point, const glm::vec3& planeNormal,
                                    const glm::vec3& planePoint);
    
    glm::vec3 ProjectPointOntoLine(const glm::vec3& point, const glm::vec3& lineStart,
                                   const glm::vec3& lineDir);
};

#endif