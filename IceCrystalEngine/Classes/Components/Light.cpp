#include <Ice/Components/Light.h>

#include <Ice/Core/LightingManager.h>
#include <Ice/Components/Camera.h>

DirectionalLight::DirectionalLight() : Component() 
{
	Initialize();
}

DirectionalLight::DirectionalLight(glm::vec3 color, float strength) : Component()
{
	this->color = color;
	this->strength = strength;

	Initialize();
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

glm::mat4 DirectionalLight::GetLightSpaceMatrix()
{
	float nearPlane = 0.5f, farPlane = 100.0f;
	glm::mat4 lightProjection = glm::ortho(-frustumSize, frustumSize, -frustumSize, frustumSize, nearPlane, farPlane);

	Camera* mainCamera = sceneManager.mainCamera;

	glm::mat4 lightView = glm::lookAt(mainCamera->transform->position - (transform->forward * 10.0f), mainCamera->transform->position - (transform->forward * 9.0f), transform->up);
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