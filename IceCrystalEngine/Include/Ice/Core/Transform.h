#pragma once

#ifndef TRANSFORM_H

#define TRANSFORM_H

#include <Ice/Core/Actor.h>
#include <glm/glm.hpp>

class Transform
{

public:

	Actor* actor;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	Transform(Actor* owner);

	// Helper Functions
	void Translate(glm::vec3 translation);
	void Translate(float x, float y, float z);
	
	void Rotate(glm::vec3 rotation);
	void Rotate(float x, float y, float z);

	void Scale(glm::vec3 scale);
	void Scale(float x, float y, float z);

};


#endif