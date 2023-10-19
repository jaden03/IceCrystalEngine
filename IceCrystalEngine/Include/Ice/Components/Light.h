#pragma once

#ifndef LIGHT_H

#define LIGHT_H

#include <glm/glm.hpp>
#include <Ice/Core/Component.h>

class DirectionalLight : public Component
{
	void Initialize();
	
public:
	
	DirectionalLight();
	DirectionalLight(glm::vec3 color, float strength);

	~DirectionalLight();


	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	
	int shadowMapResolution = 1024;
	bool castShadows = true;
	unsigned int depthMap;
	
	
	glm::mat4 GetLightSpaceMatrix();
	
};


class PointLight : public Component
{

public:

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	float radius = 5;

	PointLight();
	PointLight(glm::vec3 color, float strength, float radius);

	~PointLight();
	
};

#endif