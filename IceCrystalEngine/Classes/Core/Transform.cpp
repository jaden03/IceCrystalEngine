#include <Ice/Core/Transform.h>

Transform::Transform(Actor* owner)
{
	actor = owner;

	position = glm::vec3(0.0f, 0.0f, 0.0f);
	rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);
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

void Transform::Rotate(glm::vec3 rotation)
{
	this->rotation += rotation;
}
void Transform::Rotate(float x, float y, float z)
{
	this->rotation += glm::vec3(x, y, z);
}

void Transform::Scale(glm::vec3 scale)
{
	this->scale += scale;
}
void Transform::Scale(float x, float y, float z)
{
	this->scale += glm::vec3(x, y, z);
}