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
	~Transform();

};


#endif