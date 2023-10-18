#pragma once

#ifndef LIGHT_H

#define LIGHT_H

#include <glm/glm.hpp>
#include <Ice/Core/Component.h>

class DirectionalLight : public Component
{
public:

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	
	DirectionalLight();
	DirectionalLight(glm::vec3 color, float strength);

	~DirectionalLight();
	
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