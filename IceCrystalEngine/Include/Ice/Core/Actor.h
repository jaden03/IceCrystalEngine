#pragma once

#ifndef ACTOR_H

#define ACTOR_H

#include <vector>

class Transform; // forward declaration because actor and transform depend on eachother
class Component;

class Actor
{

public:
	
	Transform* transform;
	std::vector<Component*>* components;

	Actor();
	~Actor();

	// Add Component by Type
	template <typename T>
	T* AddComponent();
	
	// Get Component by Type
	template <typename T>
	T* GetComponent();

	
	// Remove Component by Pointer
	void RemoveComponent(Component* component);
	
	// Delete Component by Pointer
	void DeleteComponent(Component* component);

	// Add Component by Pointer
	void AddComponent(Component* component);

};

#endif