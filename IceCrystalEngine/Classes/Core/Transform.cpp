#include <Ice/Core/Transform.h>

Transform::Transform(Actor* owner)
{
	actor = owner;

	position = glm::vec3(0.0f, 0.0f, 0.0f);
	rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	right = glm::vec3(1.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
}


// Update
void Transform::Update()
{
	forward = Forward();
	right = Right();
	up = Up();

	rotation = glm::quat(glm::vec3(eulerAngles.x, eulerAngles.y, eulerAngles.z));
}




// Helper Functions
void Transform::Translate(glm::vec3 translation)
{
	position += translation;
}
void Transform::Translate(float x, float y, float z)
{
	position += glm::vec3(x, y, z);
}
void Transform::TranslateDelta(glm::vec3 translation)
{
	position += translation * sceneManager.deltaTime;
}
void Transform::TranslateDelta(float x, float y, float z)
{
	position += glm::vec3(x, y, z) * sceneManager.deltaTime;
}

void Transform::Rotate(glm::vec3 rotation)
{
	this->eulerAngles += rotation;
}
void Transform::Rotate(float x, float y, float z)
{
	this->eulerAngles += glm::vec3(x, y, z);
}
void Transform::RotateDelta(glm::vec3 rotation)
{
	this->eulerAngles += rotation * sceneManager.deltaTime;
}
void Transform::RotateDelta(float x, float y, float z)
{
	this->eulerAngles += glm::vec3(x, y, z) * sceneManager.deltaTime;
}

void Transform::Scale(glm::vec3 scale)
{
	this->scale += scale;
}
void Transform::Scale(float x, float y, float z)
{
	this->scale += glm::vec3(x, y, z);
}
void Transform::ScaleDelta(glm::vec3 scale)
{
	this->scale += scale * sceneManager.deltaTime;
}
void Transform::ScaleDelta(float x, float y, float z)
{
	this->scale += glm::vec3(x, y, z) * sceneManager.deltaTime;
}


glm::vec3 Transform::Forward()
{
	return glm::vec3(0.0f, 0.0f, 1.0f) * rotation;
}

glm::vec3 Transform::Right()
{
	return glm::vec3(1.0f, 0.0f, 0.0f) * rotation;
}

glm::vec3 Transform::Up()
{
	return glm::vec3(0.0f, 1.0f, 0.0f) * rotation;
}