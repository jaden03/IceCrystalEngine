#pragma once

#ifndef COMPONENT_H

#define COMPONENT_H

#include <Ice/Core/Transform.h>

class Component
{

public:
	Actor* owner;
	Transform* transform;
	bool enabled = true;

	Component();
	virtual ~Component() = default;

	// Called when the component is added to an actor (reparenting counts)
	virtual void Ready();
	
	// Called every frame (only fires when "attached" to an actor and component is enabled)
	virtual void Update(); // this needs to be virtual to allow it to be overridden by derived classes

	// Called every frame but later than Update (only fires when component is enabled)
	virtual void LateUpdate();

	// Called at the end of every frame with no depth testing or culling (only fires when component is enabled)
	virtual void OverlayUpdate();

	// Enable/disable the component
	void SetEnabled(bool value) { enabled = value; }
	bool IsEnabled() const { return enabled; }

};

#endif