#pragma once

#ifndef FREECAM_H

#define FREECAM_H

#include <Ice/Managers/SceneManager.h>
#include <Ice/Core/Input.h>

#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <Ice/Components/Rendering/Renderer.h>
#include <Ice/Components/Camera.h>

#include <Ice/Core/Component.h>

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

	bool requireRightClick = true;
	
	Freecam();

	void Ready() override;
	void Update() override;

};

#endif