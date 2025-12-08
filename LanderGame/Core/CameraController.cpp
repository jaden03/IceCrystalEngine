#include "CameraController.h"
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Input.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

CameraController::CameraController() : Component()
{
}

void CameraController::Ready()
{
    // Get the target actor
    targetActor = SceneManager::GetInstance().GetActorByTag(targetTag);
    // Initialize last mouse position
    lastMousePos = Input::GetMousePosition();
}

void CameraController::Update()
{
    if (targetActor == nullptr)
        return;

    Input& input = Input::GetInstance();

    // Get mouse input
    glm::vec2 currentMousePos = Input::GetMousePosition();
    float mouseDeltaX = currentMousePos.x - lastMousePos.x;
    float mouseDeltaY = currentMousePos.y - lastMousePos.y;
    lastMousePos = currentMousePos;

    // Update target rotation
    targetYaw += mouseDeltaX * mouseSensitivity;
    targetPitch -= mouseDeltaY * mouseSensitivity;

    // Clamp pitch
    if (targetPitch > maxPitch)
        targetPitch = maxPitch;
    else if (targetPitch < minPitch)
        targetPitch = minPitch;

    // Update distance
    if (Input::scrolledUp)
        cameraDistance -= 0.5f;
    else if (Input::scrolledDown)
        cameraDistance += 0.5f;

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

    // Make camera look at target with stable up vector (using LookRotation pattern)
    glm::vec3 direction = glm::normalize(targetPos - transform->position);
    glm::vec3 up = localUp;
    
    // Use glm::lookAt like your working Lua code
    glm::vec3 fakePos = glm::vec3(0, 0, 0);
    glm::mat4 viewMatrix = glm::lookAt(fakePos, fakePos - direction, up);
    
    transform->rotation = glm::quat_cast(viewMatrix);
    transform->eulerAngles = glm::degrees(glm::eulerAngles(transform->rotation));
}