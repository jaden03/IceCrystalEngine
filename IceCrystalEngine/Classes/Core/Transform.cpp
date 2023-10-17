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

void Transform::Rotate(glm::vec3 rot)
{
	eulerAngles += rot;
	rotation = glm::quat(eulerAngles);
}
void Transform::Rotate(float x, float y, float z)
{
	eulerAngles += glm::vec3(x, y, z);
	rotation = glm::quat(eulerAngles);
}
void Transform::RotateDelta(glm::vec3 rot)
{
	eulerAngles += rot * sceneManager.deltaTime;
	rotation = glm::quat(eulerAngles);
}
void Transform::RotateDelta(float x, float y, float z)
{
	eulerAngles += glm::vec3(x, y, z) * sceneManager.deltaTime;
	rotation = glm::quat(eulerAngles);
}

void Transform::Scale(glm::vec3 scale)
{
	scale += scale;
}
void Transform::Scale(float x, float y, float z)
{
	scale += glm::vec3(x, y, z);
}
void Transform::ScaleDelta(glm::vec3 scale)
{
	scale += scale * sceneManager.deltaTime;
}
void Transform::ScaleDelta(float x, float y, float z)
{
	scale += glm::vec3(x, y, z) * sceneManager.deltaTime;
}

void Transform::LookAt(float x, float y, float z)
{
	glm::vec3 target = glm::vec3(x, y, z);
	glm::vec3 direction = glm::normalize(target - position);

	rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	eulerAngles = glm::eulerAngles(rotation);
}

void Transform::LookAt(glm::vec3 target)
{
	glm::vec3 direction = glm::normalize(target - position);

	rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	eulerAngles = glm::eulerAngles(rotation);
}




void Transform::SetRotation(glm::vec3 rot)
{
	eulerAngles = rot;
	rotation = glm::quat(eulerAngles);
}
void Transform::SetRotation(float x, float y, float z)
{
	eulerAngles = glm::vec3(x, y, z);
	rotation = glm::quat(eulerAngles);
}
void Transform::SetRotation(glm::quat rot)
{
	eulerAngles = glm::eulerAngles(rot);
	rotation = rot;
}



glm::vec3 Transform::Forward()
{
	return glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f) * rotation);
}

glm::vec3 Transform::Right()
{
	return glm::normalize(glm::vec3(-1.0f, 0.0f, 0.0f) * rotation);
}

glm::vec3 Transform::Up()
{
	return glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f) * rotation);
}