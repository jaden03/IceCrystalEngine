#ifdef ICE_EDITOR

#include <Ice/Editor/EditorCamera.h>
#include <Ice/Core/Input.h>
#include <iostream>

EditorCamera::EditorCamera()
    : position(0.0f, 5.0f, 10.0f)
    , forward(0.0f, 0.0f, -1.0f)
    , up(0.0f, 1.0f, 0.0f)
    , right(1.0f, 0.0f, 0.0f)
    , fov(45.0f)
    , nearPlane(0.1f)
    , farPlane(1000.0f)
    , aspectRatio(16.0f / 9.0f)
    , yaw(-90.0f)
    , pitch(0.0f)
    , moveSpeed(5.0f)
    , lookSpeed(0.15f)
    , boostMultiplier(3.0f)
    , lastMousePos(0.0f, 0.0f)
    , wasRightMouseDown(false)
    , isViewportFocused(false)
    , firstMouseInput(true)
{
    view = glm::mat4(1.0f);
    projection = glm::mat4(1.0f);
    UpdateVectors();
}

EditorCamera::~EditorCamera()
{
}

void EditorCamera::Initialize(const glm::vec3& initialPosition)
{
    position = initialPosition;
    yaw = -90.0f;
    pitch = 0.0f;
    
    // Reset input state
    lastMousePos = glm::vec2(0.0f, 0.0f);
    wasRightMouseDown = false;
    isViewportFocused = false;
    firstMouseInput = true;
    
    UpdateVectors();
    UpdateViewMatrix();
    UpdateProjectionMatrix(1280.0f, 720.0f);
    
    std::cout << "[EditorCamera] Initialized at position (" 
              << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    std::cout << "[EditorCamera] Freecam mode enabled" << std::endl;
}

void EditorCamera::Update(float deltaTime, bool isMouseInViewport, const glm::vec2& mousePos)
{
    Input& input = Input::GetInstance();
    
    // Automatically focus viewport when mouse is inside it
    isViewportFocused = isMouseInViewport;
    
    // Current mouse button states
    bool isRightMouseDown = input.GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT);
    
    // Calculate mouse delta
    glm::vec2 mouseDelta = mousePos - lastMousePos;
    
    // Reset on first input or when mouse button is first pressed to prevent jumps
    if (firstMouseInput || (isRightMouseDown && !wasRightMouseDown))
    {
        mouseDelta = glm::vec2(0.0f);
        firstMouseInput = false;
    }
    
    // Process input when mouse is in viewport
    if (isMouseInViewport)
    {
        // Right mouse button - Free look (only rotate while RMB is held)
        if (isRightMouseDown && wasRightMouseDown)
        {
            // Only apply rotation if there's meaningful delta
            if (glm::length(mouseDelta) > 0.1f)
            {
                ProcessMouseMovement(mouseDelta.x, mouseDelta.y);
            }
        }
        
        // WASD keyboard movement (always active when mouse in viewport)
        glm::vec3 moveDirection(0.0f);
        
        if (input.GetKey(GLFW_KEY_W))
            moveDirection += forward;
        if (input.GetKey(GLFW_KEY_S))
            moveDirection -= forward;
        if (input.GetKey(GLFW_KEY_D))
            moveDirection += right;
        if (input.GetKey(GLFW_KEY_A))
            moveDirection -= right;
        if (input.GetKey(GLFW_KEY_E) || input.GetKey(GLFW_KEY_SPACE))
            moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);  // World up
        if (input.GetKey(GLFW_KEY_Q) || input.GetKey(GLFW_KEY_LEFT_CONTROL))
            moveDirection -= glm::vec3(0.0f, 1.0f, 0.0f);  // World down
        
        if (glm::length(moveDirection) > 0.001f)
        {
            bool boost = input.GetKey(GLFW_KEY_LEFT_SHIFT) || input.GetKey(GLFW_KEY_RIGHT_SHIFT);
            ProcessKeyboardMovement(moveDirection, deltaTime, boost);
        }
        
        // Scroll wheel - Adjust movement speed
        if (Input::scrolledUp)
        {
            moveSpeed += 1.0f;
            moveSpeed = glm::clamp(moveSpeed, 1.0f, 50.0f);
            std::cout << "[EditorCamera] Speed: " << moveSpeed << std::endl;
        }
        else if (Input::scrolledDown)
        {
            moveSpeed -= 1.0f;
            moveSpeed = glm::clamp(moveSpeed, 1.0f, 50.0f);
            std::cout << "[EditorCamera] Speed: " << moveSpeed << std::endl;
        }
    }
    
    // Update tracking variables for next frame
    lastMousePos = mousePos;
    wasRightMouseDown = isRightMouseDown;
    
    // Always update view matrix
    UpdateViewMatrix();
}

void EditorCamera::UpdateViewMatrix()
{
    view = glm::lookAt(position, position + forward, up);
}

void EditorCamera::UpdateProjectionMatrix(float width, float height)
{
    if (height > 0.0f)
    {
        aspectRatio = width / height;
        projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    }
}

void EditorCamera::UpdateVectors()
{
    // Calculate forward vector from yaw and pitch
    glm::vec3 newForward;
    newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newForward.y = sin(glm::radians(pitch));
    newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    forward = glm::normalize(newForward);
    
    // Calculate right and up vectors
    right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, forward));
}

void EditorCamera::ProcessMouseMovement(float deltaX, float deltaY)
{
    yaw += deltaX * lookSpeed;
    pitch -= deltaY * lookSpeed;
    
    // Constrain pitch to prevent camera flipping
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    
    // Keep yaw in 0-360 range (optional, for cleaner values)
    if (yaw > 360.0f) yaw -= 360.0f;
    if (yaw < 0.0f) yaw += 360.0f;
    
    UpdateVectors();
}

void EditorCamera::ProcessKeyboardMovement(const glm::vec3& direction, float deltaTime, bool boost)
{
    glm::vec3 normalizedDirection = glm::normalize(direction);
    float speed = moveSpeed * deltaTime;
    
    if (boost)
        speed *= boostMultiplier;
    
    position += normalizedDirection * speed;
}

void EditorCamera::FocusOn(const glm::vec3& focusPoint)
{
    // Calculate direction to focus point
    glm::vec3 directionToTarget = glm::normalize(focusPoint - position);
    
    // Calculate yaw and pitch from direction
    yaw = glm::degrees(atan2(directionToTarget.z, directionToTarget.x));
    pitch = glm::degrees(asin(directionToTarget.y));
    
    UpdateVectors();
    UpdateViewMatrix();
    
    std::cout << "[EditorCamera] Focused on point (" 
              << focusPoint.x << ", " << focusPoint.y << ", " << focusPoint.z << ")" << std::endl;
}

void EditorCamera::Reset()
{
    position = glm::vec3(0.0f, 5.0f, 10.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    moveSpeed = 5.0f;
    
    // Reset input state
    lastMousePos = glm::vec2(0.0f, 0.0f);
    wasRightMouseDown = false;
    firstMouseInput = true;
    
    UpdateVectors();
    UpdateViewMatrix();

    std::cout << "[EditorCamera] Reset to default position" << std::endl;
}

#endif // ICE_EDITOR