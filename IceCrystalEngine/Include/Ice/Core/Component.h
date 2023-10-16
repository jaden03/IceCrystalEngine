#pragma once

#ifndef COMPONENT_H

#define COMPONENT_H

#include <Ice\Core\Actor.h>

class Component
{

public:

	Actor* actor;

	Component(Actor* owner);
	~Component();

};

#endif