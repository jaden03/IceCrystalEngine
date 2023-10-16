#pragma once

#ifndef TRANSFORM_H

#define TRANSFORM_H

#include <Ice/Core/Actor.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Ice/Core/SceneManager.h>

class Transform
{

public:

	Actor* actor;
	SceneManager& sceneManager = SceneManager::GetInstance();

	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 eulerAngles;
	glm::vec3 scale;

	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	Transform(Actor* owner);

	// Update (dont use)
	void Update();
	
	// Helper Functions
	
	void Translate(glm::vec3 translation);
	void Translate(float x, float y, float z);
	void TranslateDelta(glm::vec3 translation);
	void TranslateDelta(float x, float y, float z);
	
	void Rotate(glm::vec3 rotation);
	void Rotate(float x, float y, float z);
	void RotateDelta(glm::vec3 rotation);
	void RotateDelta(float x, float y, float z);

	// If you actually find a use for these, your weird
	void Scale(glm::vec3 scale);
	void Scale(float x, float y, float z);
	void ScaleDelta(glm::vec3 scale);
	void ScaleDelta(float x, float y, float z);

private:

	glm::vec3 Forward();
	glm::vec3 Right();
	glm::vec3 Up();

};


#endif