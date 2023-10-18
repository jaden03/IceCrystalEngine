#include <Ice/Components/Light.h>
#include <Ice/Core/LightingManager.h>

LightingManager& lightingManager = LightingManager::GetInstance();

DirectionalLight::DirectionalLight() : Component() {}

DirectionalLight::DirectionalLight(glm::vec3 color, float strength) : Component()
{
	this->color = color;
	this->strength = strength;

	lightingManager.AddDirectionalLight(this);
}

DirectionalLight::~DirectionalLight()
{
	lightingManager.RemoveDirectionalLight(this);
}

glm::mat3 DirectionalLight::GetLightData()
{
	//[dx] [dy] [dx]
	//[r] [g] [b]
	//[strength] [] []

	glm::mat3 dataMatrix;
	
	// Direction
	dataMatrix[0].x = transform->forward.x;
	dataMatrix[0].y = transform->forward.y;
	dataMatrix[0].z = transform->forward.z;

	// Color
	dataMatrix[1].x = color.x;
	dataMatrix[1].y = color.y;
	dataMatrix[1].z = color.z;

	// Strength
	dataMatrix[2].x = strength;

	return dataMatrix;
}






PointLight::PointLight() : Component() {}

PointLight::PointLight(glm::vec3 color, float strength, float radius) : Component()
{
	this->color = color;
	this->strength = strength;
	this->radius = radius;

	lightingManager.AddPointLight(this);
}

PointLight::~PointLight()
{
	lightingManager.RemovePointLight(this);
}

glm::mat3 PointLight::GetLightData()
{
	//[x] [y] [x]
	//[r] [g] [b]
	//[strength] [radius] []

	glm::mat3 dataMatrix;

	// Direction
	dataMatrix[0].x = transform->position.x;
	dataMatrix[0].y = transform->position.y;
	dataMatrix[0].z = transform->position.z;

	// Color
	dataMatrix[1].x = color.x;
	dataMatrix[1].y = color.y;
	dataMatrix[1].z = color.z;

	// Strength + Radius
	dataMatrix[2].x = strength;
	dataMatrix[2].y = radius;

	return dataMatrix;
}