#pragma once

#ifndef CAMERA_H

#define CAMERA_H

#include <glm/glm.hpp>

#include <Ice/Core/Component.h>

class Camera : public Component
{
	WindowManager& windowManager = WindowManager::GetInstance();
	
public:

	Camera();
	Camera(float fieldOfView);
	Camera(float fieldOfView, float nearClippingPlane, float farClippingPlane);
	
	float fieldOfView = 90.0f;
	float nearClippingPlane = 0.01f;
	float farClippingPlane = 1000.0f;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	void Update() override;
};

#endif