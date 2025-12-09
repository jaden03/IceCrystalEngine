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