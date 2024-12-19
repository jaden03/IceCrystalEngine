#pragma once

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <Ice/Core/Component.h>

#include "glad/glad.h"

class Camera;

struct DirectionalLightData {
	glm::vec3 direction;
	glm::vec3 color;
	float strength;
	GLuint shadowMap;
	glm::mat4 lightSpaceMatrix;
	bool castShadows;
};
class DirectionalLight : public Component
{

	void Initialize() override;
	
public:
	
	DirectionalLight();
	DirectionalLight(glm::vec3 color, float strength);

	~DirectionalLight();


	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	
	// this essentially is the shadow distance, but the shadows will be lower quality unless you increase the shadowMapResolution
	float frustumSize = 15.0f;
	float frustumNearPlane = 1.0f;
	float frustumFarPlane = 100.0f;
	// this is the resolution of the shadow map, the higher the resolution, the better the shadows will look, but the more performance it will cost
	int shadowMapResolution = 2048;
	bool castShadows = true;
	GLuint depthMap;
	
	
	glm::mat4 GetLightSpaceMatrix(Camera* relativeCamera);
	
};


struct PointLightData {
	glm::vec3 position;
	glm::vec3 color;
	float strength;
	float radius;
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


struct SpotLightData {
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 color;
	float strength;
	float angle;
	float outerAngle;
	float distance;
	GLuint shadowMap;
	glm::mat4 lightSpaceMatrix;
	bool castShadows;
};
class SpotLight : public Component
{

	void Initialize() override;
	
public: 

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float strength = 1;
	float distance = 10;
	float angle = 25;
	
	SpotLight();
	SpotLight(glm::vec3 color, float strength, float distance, float angle);
	
	~SpotLight();



	int shadowMapResolution = 1024;
	bool castShadows = true;
	GLuint depthMap;
	
	glm::mat4 GetLightSpaceMatrix();
};

#endif