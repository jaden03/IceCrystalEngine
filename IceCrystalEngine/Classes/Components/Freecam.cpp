#include <Ice/Components/Freecam.h>

#include <iostream>

#include <GLFW/glfw3.h>

Freecam::Freecam() : Component()
{
    speed = 5.0f;
    sensitivity = 0.1f;

    pitch = 0.0f;
    yaw = 0.0f;
    roll = 0.0f;
    
    input.GetMousePosition(&lastMouseX, &lastMouseY);
}


void Freecam::Ready()
{
    input.CreateAxis("Horizontal", GLFW_KEY_D, GLFW_KEY_A);
    input.CreateAxis("Vertical", GLFW_KEY_W, GLFW_KEY_S);
}

void Freecam::Update()
{
    double mouseX, mouseY;
    input.GetMousePosition(&mouseX, &mouseY);

    if (requireRightClick)
    {
        input.lockCursor = false;

        if (!input.GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
        {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            return;
        }
    }
    else
    {
        input.lockCursor = true;
    }
    
    float horizontal = input.GetAxis("Horizontal");
    float vertical = input.GetAxis("Vertical");

    // Move the camera
    glm::vec3 movement = (vertical * transform->forward) + (-horizontal * transform->right);

    // Up and down
    if (input.GetKey(GLFW_KEY_SPACE))
        movement += glm::vec3(0, 1, 0);
    if (input.GetKey(GLFW_KEY_LEFT_CONTROL))
        movement -= glm::vec3(0, 1, 0);

    float currentSpeed = input.GetKey(GLFW_KEY_LEFT_SHIFT) ? speed * 1.5f : speed;
    currentSpeed = input.GetKey(GLFW_KEY_LEFT_ALT) ? speed * 0.2f : currentSpeed;

    if (glm::length(movement) != 0.0f)
        movement = glm::normalize(movement) * currentSpeed;
    
    transform->TranslateDelta(movement);

    // Rotate the camera
    float deltaX = mouseX - lastMouseX;
    float deltaY = mouseY - lastMouseY;

    yaw -= deltaX * sensitivity;
    pitch += deltaY * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Build rotation from pitch and yaw using quaternions
    glm::quat pitchQuat = glm::angleAxis(glm::radians(pitch), glm::vec3(1, 0, 0));
    glm::quat yawQuat = glm::angleAxis(glm::radians(yaw), glm::vec3(0, 1, 0));
    
    transform->rotation = yawQuat * pitchQuat;

    lastMouseX = mouseX;
    lastMouseY = mouseY;
}