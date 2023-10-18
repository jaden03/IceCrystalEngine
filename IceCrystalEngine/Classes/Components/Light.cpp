#include <Ice/Components/Light.h>

#include <Ice/Core/LightingManager.h>

DirectionalLight::DirectionalLight() : Component() 
{
	LightingManager::GetInstance().AddDirectionalLight(this);
}

DirectionalLight::DirectionalLight(glm::vec3 color, float strength) : Component()
{
	this->color = color;
	this->strength = strength;

	LightingManager::GetInstance().AddDirectionalLight(this);
}

DirectionalLight::~DirectionalLight()
{
	LightingManager::GetInstance().RemoveDirectionalLight(this);
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