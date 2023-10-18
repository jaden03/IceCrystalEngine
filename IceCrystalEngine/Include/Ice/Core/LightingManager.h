#pragma once

#ifndef LIGHTING_MANAGER_H

#define LIGHTING_MANAGER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

class DirectionalLight;
class PointLight;

class LightingManager
{

public:

	static LightingManager& GetInstance()
	{
		static LightingManager instance; // Static local variable ensures a single instance
		return instance;
	}

	float ambientLightingStrength = 0.15f;
	glm::vec3 ambientLightingColor = glm::vec3(1.0f, 1.0f, 1.0f);

	int maxDirectionalLights = 5;
	int maxPointLights = 128;

	void InitializeLighting();

	void UpdateLights();

	void AddDirectionalLight(DirectionalLight* light);
	void AddPointLight(PointLight* light);

	void RemoveDirectionalLight(DirectionalLight* light);
	void RemovePointLight(PointLight* light);

private:

	unsigned int directionalLightSSBO;
	unsigned int pointLightSSBO;
	

	LightingManager(); // Private constructor to ensure a single instance
	//~SceneInitializer();

	LightingManager(LightingManager const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(LightingManager const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif