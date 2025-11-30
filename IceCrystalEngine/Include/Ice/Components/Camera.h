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
	float farClippingPlane = 50.0f;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	std::vector<glm::vec4> frustumCorners;
	glm::vec3 frustumCenter;

	std::array<glm::vec4, 8> GetFrustomCornersWorldSpace(float nearDist, float farDist);

	void Update() override;

private:
	std::vector<glm::vec4> GetFrustomCorners();
};

#endif