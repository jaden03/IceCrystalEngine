#include <iostream>
#include <ostream>
#include <Ice/Core/Transform.h>

#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/string_cast.hpp"

SceneManager& sceneManager = SceneManager::GetInstance();

Transform::Transform(Actor* owner)
{
	actor = owner;

	position = glm::vec3(0.0f, 0.0f, 0.0f);
	rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	eulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	localPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	localRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	localEulerAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	localScale = glm::vec3(1.0f, 1.0f, 1.0f);


	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	right = glm::vec3(1.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
}


// Update
void Transform::Update()
{
	// Update direction vectors from current rotation
	forward = Forward();
	right = Right();
	up = Up();

	// If we have a parent, calculate world position/rotation from local + parent
	if (parent != nullptr)
	{
		// World rotation = parent rotation * local rotation
		rotation = parent->rotation * localRotation;
        
		// Recalculate direction vectors after rotation update
		forward = Forward();
		right = Right();
		up = Up();
        
		// World position = parent position + local offset rotated by parent
		position = parent->position 
			+ parent->Forward() * localPosition.z 
			+ parent->Right() * localPosition.x 
			+ parent->Up() * localPosition.y;
	}

	// Now update all children (they depend on our updated values)
	for (int i = 0; i < children->size(); i++)
	{
		children->at(i)->Update();
	}
}


void Transform::SetParent(Transform* parent)
{
	if (this->parent != nullptr)
	{
		// remove from old parent
		for (int i = 0; i < this->parent->children->size(); i++)
		{
			if (this->parent->children->at(i) == this)
			{
				this->parent->children->erase(this->parent->children->begin() + i);
			}
		}
	}

	// get the offset from the new parent
	glm::vec3 offset = position - parent->position;
	// set the localPosition to the offset
	localPosition = offset;

	// add to new parent
	this->parent = parent;
	this->parent->children->push_back(this);
}




// Additive Transformations
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
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}
void Transform::Rotate(float x, float y, float z)
{
	eulerAngles += glm::vec3(x, y, z);
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}
void Transform::RotateDelta(glm::vec3 rot)
{
	eulerAngles += rot * sceneManager.deltaTime;
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}
void Transform::RotateDelta(float x, float y, float z)
{
	eulerAngles += glm::vec3(x, y, z) * sceneManager.deltaTime;
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}

void Transform::Scale(glm::vec3 _scale)
{
	scale += _scale;
}
void Transform::Scale(float x, float y, float z)
{
	scale += glm::vec3(x, y, z);
}
void Transform::ScaleDelta(glm::vec3 _scale)
{
	scale += _scale * sceneManager.deltaTime;
}
void Transform::ScaleDelta(float x, float y, float z)
{
	scale += glm::vec3(x, y, z) * sceneManager.deltaTime;
}
// ----------------- \\



// Absolute Transformations
void Transform::SetPosition(glm::vec3 pos)
{
	position = pos;
}
void Transform::SetPosition(float x, float y, float z)
{
	position = glm::vec3(x, y, z);
}

void Transform::SetRotation(glm::vec3 rot)
{
	eulerAngles = rot;
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}
void Transform::SetRotation(float x, float y, float z)
{
	eulerAngles = glm::vec3(x, y, z);
	rotation = glm::quat(glm::vec3(glm::radians(eulerAngles)));
}
void Transform::SetRotation(glm::quat rot)
{
	rotation = rot;
    
	// Extract with explicit order matching your engine's convention
	glm::vec3 radians;
	glm::extractEulerAngleXYZ(glm::mat4_cast(rotation), radians.x, radians.y, radians.z);
	eulerAngles = glm::degrees(radians);
}

void Transform::SetScale(glm::vec3 scale)
{
	this->scale = scale;
}
void Transform::SetScale(float x, float y, float z)
{
	this->scale = glm::vec3(x, y, z);
}



// Additive Local Transformations
void Transform::TranslateLocal(glm::vec3 translation)
{
	localPosition += translation;
}

void Transform::TranslateLocal(float x, float y, float z)
{
	localPosition += glm::vec3(x, y, z);
}

void Transform::TranslateLocalDelta(glm::vec3 translation)
{
	localPosition += translation * sceneManager.deltaTime;
}

void Transform::TranslateLocalDelta(float x, float y, float z)
{
	localPosition += glm::vec3(x, y, z) * sceneManager.deltaTime;
}

void Transform::RotateLocal(glm::vec3 rot)
{
	localEulerAngles += rot;
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::RotateLocal(float x, float y, float z)
{
	localEulerAngles += glm::vec3(x, y, z);
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::RotateLocalDelta(glm::vec3 rot)
{
	localEulerAngles += rot * sceneManager.deltaTime;
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::RotateLocalDelta(float x, float y, float z)
{
	localEulerAngles += glm::vec3(x, y, z) * sceneManager.deltaTime;
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::ScaleLocal(glm::vec3 scale)
{
	localScale += scale;
}

void Transform::ScaleLocal(float x, float y, float z)
{
	localScale += glm::vec3(x, y, z);
}

void Transform::ScaleLocalDelta(glm::vec3 scale)
{
	localScale += scale * sceneManager.deltaTime;
}

void Transform::ScaleLocalDelta(float x, float y, float z)
{
	localScale += glm::vec3(x, y, z) * sceneManager.deltaTime;
}
// ----------------- \\

// Absolute Local Transformations
void Transform::SetLocalPosition(glm::vec3 pos)
{
	localPosition = pos;
}

void Transform::SetLocalPosition(float x, float y, float z)
{
	localPosition = glm::vec3(x, y, z);
}

void Transform::SetLocalRotation(glm::vec3 rot)
{
	localEulerAngles = rot;
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::SetLocalRotation(float x, float y, float z)
{
	localEulerAngles = glm::vec3(x, y, z);
	localRotation = glm::quat(glm::radians(localEulerAngles));
}

void Transform::SetLocalRotation(glm::quat rot)
{
	localRotation = rot;
	localEulerAngles = glm::degrees(glm::eulerAngles(localRotation));
}

void Transform::SetLocalScale(glm::vec3 scale)
{
	localScale = scale;
}

void Transform::SetLocalScale(float x, float y, float z)
{
	localScale = glm::vec3(x, y, z);
}


void Transform::LookAt(glm::vec3 target)
{
	LookAt(target, glm::vec3(0, 1, 0));
}

void Transform::LookAt(float x, float y, float z)
{
	LookAt(glm::vec3(x, y, z), glm::vec3(0, 1, 0));
}

void Transform::LookAt(glm::vec3 target, glm::vec3 up)
{
	glm::vec3 forward = glm::normalize(target - position);
    
	// Handle edge case: looking parallel to up vector
	if (glm::abs(glm::dot(forward, up)) > 0.999f)
	{
		// Use world X as fallback, then derive up from that
		glm::vec3 right = glm::normalize(glm::cross(up, forward));
		if (glm::length(right) < 0.001f)
		{
			right = glm::vec3(1, 0, 0);
		}
		up = glm::normalize(glm::cross(forward, right));
	}
    
	glm::vec3 right = glm::normalize(glm::cross(up, forward));
	glm::vec3 correctedUp = glm::cross(forward, right);
    
	glm::mat3 rotMatrix(right, correctedUp, forward);
	rotation = glm::quat_cast(rotMatrix);
}


// Direction Vectors
glm::vec3 Transform::Forward()
{
	return glm::normalize(rotation * glm::vec3(0.0f, 0.0f, 1.0f));
}

glm::vec3 Transform::Right()
{
	return glm::normalize(rotation * glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 Transform::Up()
{
	return glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f));
}
// ----------------- \\