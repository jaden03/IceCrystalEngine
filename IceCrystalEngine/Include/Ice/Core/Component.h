#pragma once

#ifndef COMPONENT_H

#define COMPONENT_H

#include <Ice\Core\Actor.h>

class Component
{

public:

	Actor* owner;

	Component();
	// as in my architecture, components are "owned" by an actor they are deleted when the owner is deleted
	// therefore there is no need to have a deconstructor that deletes the owner, because the owner is already deleted

	virtual void Update(); // this needs to be virtual to allow it to be overridden by derived classes

};

#endif