#include <Ice/Components/Camera.h>

Camera::Camera() : Component()
{
}

Camera::Camera(float fieldOfView) : Component()
{
	this->fieldOfView = fieldOfView;
}

Camera::Camera(float fieldOfView, float nearClippingPlane, float farClippingPlane) : Component()
{
	this->fieldOfView = fieldOfView;
	this->nearClippingPlane = nearClippingPlane;
	this->farClippingPlane = farClippingPlane;
}

void Camera::Update()
{
	view = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
	projection = glm::perspective(glm::radians(fieldOfView), (float)windowManager.windowWidth / (float)windowManager.windowHeight, nearClippingPlane, farClippingPlane);
}