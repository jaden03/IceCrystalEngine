#include "CameraController.h"
#include <Ice/Managers/SceneManager.h>
#include <Ice/Core/Input.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <iostream>

CameraController::CameraController() : Component()
{
}

void CameraController::Ready()
{
    // Get the target actor
    targetActor = SceneManager::GetInstance().GetActorByTag(targetTag);
    
    if (targetActor == nullptr)
    {
        std::cout << "[CameraController] ERROR: Could not find actor with tag '" << targetTag << "'" << std::endl;
    }
    else
    {
        std::cout << "[CameraController] Successfully found target actor: " << targetActor->name << std::endl;
        std::cout << "[CameraController] Target position: (" << targetActor->transform->position.x 
                  << ", " << targetActor->transform->position.y 
                  << ", " << targetActor->transform->position.z << ")" << std::endl;
        
        // Calculate and set initial camera position immediately
        glm::vec3 targetPos = targetActor->transform->position;
        glm::vec3 localUp = glm::normalize(targetPos - moonCenter);
        
        // Build stable horizontal plane
        glm::vec3 worldNorth = glm::vec3(0.0f, 0.0f, 1.0f);
        if (std::abs(glm::dot(localUp, worldNorth)) > 0.9f)
        {
            worldNorth = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        
        glm::vec3 localEast = glm::normalize(glm::cross(worldNorth, localUp));
        glm::vec3 localNorth = glm::normalize(glm::cross(localUp, localEast));
        
        // Apply initial rotation
        float yawRad = glm::radians(currentYaw);
        float pitchRad = glm::radians(currentPitch);
        
        glm::vec3 horizontalDir = std::cos(yawRad) * localNorth + std::sin(yawRad) * localEast;
        glm::vec3 finalDirection = std::cos(pitchRad) * horizontalDir + std::sin(pitchRad) * localUp;
        finalDirection = glm::normalize(finalDirection);
        
        // Set initial camera position
        glm::vec3 initialPosition = targetPos - finalDirection * cameraDistance;
        transform->position = initialPosition;
        transform->LookAt(targetPos, localUp);
        
        std::cout << "[CameraController] Initial camera position set to: (" 
                  << transform->position.x << ", " 
                  << transform->position.y << ", " 
                  << transform->position.z << ")" << std::endl;
    }
    
    // Initialize last mouse position
    lastMousePos = Input::GetMousePosition();
    
    std::cout << "[CameraController] Initial camera distance: " << cameraDistance << std::endl;
    std::cout << "[CameraController] Initial yaw: " << currentYaw << ", pitch: " << currentPitch << std::endl;
}

void CameraController::Update()
{
    // Try to find target actor if we haven't found it yet
    if (targetActor == nullptr)
    {
        targetActor = SceneManager::GetInstance().GetActorByTag(targetTag);
        if (targetActor == nullptr)
        {
            static int warnCounter = 0;
            if (warnCounter % 60 == 0)  // Only warn once per second
            {
                std::cout << "[CameraController] WARNING: targetActor is still null, retrying..." << std::endl;
            }
            warnCounter++;
            return;
        }
        else
        {
            std::cout << "[CameraController] Successfully found target actor on retry: " << targetActor->name << std::endl;
        }
    }

    Input& input = Input::GetInstance();

    // Get mouse input
    glm::vec2 currentMousePos = Input::GetMousePosition();
    float mouseDeltaX = currentMousePos.x - lastMousePos.x;
    float mouseDeltaY = currentMousePos.y - lastMousePos.y;
    lastMousePos = currentMousePos;

    // Update target rotation
    if (Input::GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        targetYaw += mouseDeltaX * mouseSensitivity;
        targetPitch -= mouseDeltaY * mouseSensitivity;
    }

    // Clamp pitch
    if (targetPitch > maxPitch)
        targetPitch = maxPitch;
    else if (targetPitch < minPitch)
        targetPitch = minPitch;

    // Update distance
    if (Input::scrolledUp)
        cameraDistance--;
    else if (Input::scrolledDown)
        cameraDistance++;

    // Clamp camera distance
    if (cameraDistance < minDistance)
        cameraDistance = minDistance;
    else if (cameraDistance > maxDistance)
        cameraDistance = maxDistance;

    // Smooth rotation interpolation
    currentYaw = currentYaw + (targetYaw - currentYaw) * rotationSmoothSpeed;
    currentPitch = currentPitch + (targetPitch - currentPitch) * rotationSmoothSpeed;

    // Get target position and calculate local "up" based on position
    glm::vec3 targetPos = targetActor->transform->position;
    glm::vec3 localUp = glm::normalize(targetPos - moonCenter);
    
    // Build a stable horizontal plane perpendicular to localUp
    glm::vec3 worldNorth = glm::vec3(0.0f, 0.0f, 1.0f);
    if (std::abs(glm::dot(localUp, worldNorth)) > 0.9f)
    {
        worldNorth = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 localEast = glm::normalize(glm::cross(worldNorth, localUp));
    glm::vec3 localNorth = glm::normalize(glm::cross(localUp, localEast));

    // Apply camera rotations in this stable local space
    float yawRad = glm::radians(currentYaw);
    float pitchRad = glm::radians(currentPitch);

    // Yaw rotates around localUp, pitch rotates the result up/down
    glm::vec3 horizontalDir = std::cos(yawRad) * localNorth + std::sin(yawRad) * localEast;
    glm::vec3 finalDirection = std::cos(pitchRad) * horizontalDir + std::sin(pitchRad) * localUp;
    finalDirection = glm::normalize(finalDirection);

    // Position camera
    glm::vec3 targetPosition = targetPos - finalDirection * cameraDistance;
    transform->position = targetPosition;

    // Make camera look at target with custom up vector
    transform->LookAt(targetPos, localUp);
}