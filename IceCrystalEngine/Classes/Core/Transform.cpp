#include <Ice/Core/Transform.h>

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
	forward = Forward();
	right = Right();
	up = Up();

	if (parent != nullptr)
	{
		rotation = parent->rotation * localRotation;
		eulerAngles = glm::degrees(glm::eulerAngles(rotation));
	}
	localRotation = glm::quat(glm::radians(localEulerAngles));

	// loop through children
	for (int i = 0; i < children->size(); i++)
	{
		// update child
		Transform* child = children->at(i);
		child->SetPosition(position + glm::vec3(child->localPosition.z * forward) + glm::vec3(-child->localPosition.x * right) + glm::vec3(child->localPosition.y * up));
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
	eulerAngles = glm::degrees(glm::eulerAngles(rotation));
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












void Transform::LookAt(float x, float y, float z)
{
	glm::vec3 target = glm::vec3(x, y, z);
	glm::vec3 direction = glm::normalize(target - position);

	rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	eulerAngles = glm::degrees(glm::eulerAngles(rotation));
}

void Transform::LookAt(glm::vec3 target)
{
	glm::vec3 direction = glm::normalize(target - position);

	rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	eulerAngles = glm::degrees(glm::eulerAngles(rotation));
}






// Direction Vectors
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
// ----------------- \\