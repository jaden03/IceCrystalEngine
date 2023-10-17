#pragma once

#ifndef FREECAM_H

#define FREECAM_H

#include <Ice/Components/Renderer.h>
#include <Ice/Core/Transform.h>
#include <Ice/Core/Component.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Input.h>
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <Ice/Components/Camera.h>


class Freecam : public Component
{

	SceneManager& sceneManager = SceneManager::GetInstance();
	Input& input = Input::GetInstance();

public:

	float speed;
	float sensitivity;

	float pitch;
	float yaw;
	float roll;

	double lastMouseX;
	double lastMouseY;
	
	Freecam();

	void Ready() override;
	void Update() override;

};

#endif