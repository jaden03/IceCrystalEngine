#pragma once

#ifndef ACTOR_H

#define ACTOR_H

#include <vector>
#include <string>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/SceneManager.h>

class Transform; // forward declaration because actor and transform depend on eachother
class Component;

class Actor
{

public:
	
	std::string name = "Actor";
	std::string tag = "Untagged";

	Transform* transform;
	std::vector<Component*>* components;

	Actor();
	Actor(std::string name);
	Actor(std::string name, std::string tag);
	~Actor();

	// You have to do template stuff in the header file or else it wont compile

	// Has Component
	template <typename T>
	bool HasComponent()
	{
		for (int i = 0; i < components->size(); i++)
		{
			if (dynamic_cast<T*>(components->at(i)) != nullptr)
			{
				return true;
			}
		}
		return false;
	}
	

	// Add Component by Type
	template <typename T>
	T* AddComponent()
	{
		T* newComponent = new T;
		newComponent->owner = this;
		newComponent->transform = transform;
		newComponent->Ready();
		components->push_back(newComponent);
		return newComponent;
	}
	
	// Get Component by Type
	template <typename T>
	T* GetComponent()
	{
		for (Component* component : *components)
		{
			if (dynamic_cast<T*>(component) != nullptr)
			{
				return dynamic_cast<T*>(component);
			}
		}
		return nullptr;
	}

	
	// Remove Component by Pointer
	void RemoveComponent(Component* component);
	
	// Delete Component by Pointer
	void DeleteComponent(Component* component);

	// Add Component by Pointer
	void AddComponent(Component* component);

};

#endif