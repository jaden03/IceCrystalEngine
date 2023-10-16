#pragma once

#ifndef ACTOR_H

#define ACTOR_H

#include <vector>
#include <string>

class Transform; // forward declaration because actor and transform depend on eachother
class Component;

class Actor
{

public:
	
	std::string name;
	std::string tag;

	Transform* transform;
	std::vector<Component*>* components;

	Actor(std::string name);
	Actor(std::string name, std::string tag);
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