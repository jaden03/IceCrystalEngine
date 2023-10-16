#pragma once

#ifndef COMPONENT_H

#define COMPONENT_H

#include <Ice\Core\Actor.h>

class Component
{

public:

	Actor* owner;

	Component();
	~Component();

	virtual void Update(); // this needs to be virtual to allow it to be overridden by derived classes

};

#endif