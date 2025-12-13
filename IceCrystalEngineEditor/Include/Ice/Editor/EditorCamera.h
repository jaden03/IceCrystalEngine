#pragma once

#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

class EditorCamera
{
public:
    static EditorCamera& GetInstance();

    // Camera properties
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 right;
    
    // Matrices
    glm::mat4 view;
    glm::mat4 projection;
    
    // Camera settings
    float fov;
    float nearPlane;
    float farPlane;
    float aspectRatio;
    
    // Freecam rotation (Euler angles)
    float yaw;
    float pitch;
    
    // Movement speeds
    float moveSpeed;
    float lookSpeed;
    float boostMultiplier;
    
    // Input state tracking
    glm::vec2 lastMousePos;
    bool wasRightMouseDown;
    bool isViewportFocused;
    bool firstMouseInput;
    
    // Initialize the editor camera
    void Initialize(const glm::vec3& initialPosition = glm::vec3(0.0f, 5.0f, 10.0f));
    
    // Update camera matrices and input
    void Update(float deltaTime, bool isMouseInViewport, const glm::vec2& mousePos);
    void UpdateViewMatrix();
    void UpdateProjectionMatrix(float width, float height);
    void UpdateVectors();
    
    // Focus management
    void SetViewportFocused(bool focused);
    bool IsViewportFocused() const;
    
    // Camera controls
    void ProcessMouseMovement(float deltaX, float deltaY);
    void ProcessKeyboardMovement(const glm::vec3& direction, float deltaTime, bool boost = false);
    void FocusOn(const glm::vec3& focusPoint);
    
    // Reset camera to default position
    void Reset();
    
    // Get camera direction vectors
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

private:
    EditorCamera();
    ~EditorCamera();
    EditorCamera(EditorCamera const&) = delete;
    void operator=(EditorCamera const&) = delete;
};

#endif