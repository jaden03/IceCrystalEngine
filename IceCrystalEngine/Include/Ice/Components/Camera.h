#pragma once

#ifndef CAMERA_H

#define CAMERA_H

#include <Ice/Core/Component.h>

class Camera : public Component
{

public:

	Camera();
	Camera(float fieldOfView);
	Camera(float fieldOfView, float nearClippingPlane, float farClippingPlane);
	
	float fieldOfView = 90.0f;
	float nearClippingPlane = 0.1f;
	float farClippingPlane = 1000.0f;
	
};

#endif