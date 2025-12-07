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
    static GizmoRenderer& GetInstance()
    {
        static GizmoRenderer instance;
        return instance;
    }

    void Initialize();
    void Cleanup();

    // Render gizmos for the selected actor
    void RenderGizmos(Actor* selectedActor, Camera* camera);

    // Set the current gizmo mode
    void SetMode(GizmoMode mode) { currentMode = mode; }
    GizmoMode GetMode() const { return currentMode; }

    // Enable/disable gizmo rendering
    void SetEnabled(bool enabled) { this->enabled = enabled; }
    bool IsEnabled() const { return enabled; }

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

    bool IsDragging() const { return isDragging; }

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
    Actor* dragActor;
    glm::vec3 dragStartWorldPos;
    glm::vec3 dragStartActorPos;
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
};

#endif