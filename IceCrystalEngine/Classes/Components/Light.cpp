#include <Ice/Components/Light.h>

#include <Ice/Core/LightingManager.h>
#include <Ice/Components/Camera.h>

DirectionalLight::DirectionalLight() : Component()
{}

DirectionalLight::DirectionalLight(glm::vec3 color, float strength) : Component()
{
	this->color = color;
	this->strength = strength;
}

DirectionalLight::~DirectionalLight()
{
	LightingManager::GetInstance().RemoveDirectionalLight(this);
}

void DirectionalLight::Initialize()
{
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	LightingManager::GetInstance().AddDirectionalLight(this);
}

glm::mat4 DirectionalLight::GetLightSpaceMatrix(Camera* relativeCamera)
{
	glm::mat4 lightProjection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, frustumNearPlane, frustumFarPlane);
	
	glm::mat4 lightView = glm::lookAt(relativeCamera->transform->position - (transform->forward * 25.0f), relativeCamera->transform->position - transform->forward, transform->up);
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	return lightSpaceMatrix;
}





PointLight::PointLight() : Component() 
{
	LightingManager::GetInstance().AddPointLight(this);
}

PointLight::PointLight(glm::vec3 color, float strength, float radius) : Component()
{
	this->color = color;
	this->strength = strength;
	this->radius = radius;

	LightingManager::GetInstance().AddPointLight(this);
}

PointLight::~PointLight()
{
	LightingManager::GetInstance().RemovePointLight(this);
}




SpotLight::SpotLight() : Component()
{
	Initialize();
}

SpotLight::SpotLight(glm::vec3 color, float strength, float distance, float angle) : Component()
{
	this->color = color;
	this->strength = strength;
	this->distance = distance;
	this->angle = angle;

	Initialize();
}

SpotLight::~SpotLight()
{
	LightingManager::GetInstance().RemoveSpotLight(this);
}


void SpotLight::Initialize()
{
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	LightingManager::GetInstance().AddSpotLight(this);
}

glm::mat4 SpotLight::GetLightSpaceMatrix()
{
	glm::mat4 lightProjection = glm::perspective(glm::radians((angle + 5) * 2.0f), 1.0f, 0.1f, 100.0f);

	glm::mat4 lightView = glm::lookAt(transform->position, transform->position + transform->forward, transform->up);
	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	return lightSpaceMatrix;
}