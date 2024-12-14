#pragma once

#ifndef COMPONENT_H

#define COMPONENT_H

#include <Ice/Core/Transform.h>

class Component
{

public:

	Actor* owner;
	Transform* transform;

	Component();
	// as in my architecture, components are "owned" by an actor they are deleted when the owner is deleted
	// therefore there is no need to have a deconstructor that deletes the owner, because the owner is already deleted

	// Called when the component is added to an actor (reparenting counts)
	virtual void Ready();
	
	// Called every frame (only fires when "attached" to an actor)
	virtual void Update(); // this needs to be virtual to allow it to be overridden by derived classes

	// Called every frame but later than Update
	virtual void LateUpdate();

	// Called every frame but after rendering everything off, has no culling or depth test
	virtual void OverlayUpdate();

};

#endif