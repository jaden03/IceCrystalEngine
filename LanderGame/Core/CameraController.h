#pragma once

#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <Ice/Core/Component.h>
#include <Ice/Core/Actor.h>
#include <glm/glm.hpp>

class CameraController : public Component
{
public:
    // Camera settings
    float cameraDistance = 10.0f;
    float mouseSensitivity = 0.15f;
    float rotationSmoothSpeed = 0.01f;

    // Pitch limits
    float minPitch = -89.0f;
    float maxPitch = 89.0f;

    // Distance limits
    float minDistance = 10.0f;
    float maxDistance = 50.0f;

    // Moon center (assuming centered at origin)
    glm::vec3 moonCenter = glm::vec3(0.0f, 0.0f, 0.0f);

    // Target tag for the object to follow
    std::string targetTag = "Lander";

    CameraController();
    ~CameraController() override = default;

    void Ready() override;
    void Update() override;

private:
    // Camera rotation state (independent of target)
    float targetYaw = 180.0f;
    float targetPitch = -20.0f;
    float currentYaw = 180.0f;
    float currentPitch = -20.0f;

    glm::vec2 lastMousePos;

    // Cached reference to target actor
    Actor* targetActor = nullptr;
};

#endif