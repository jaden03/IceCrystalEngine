#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h> // include transform here for the full definition
#include <Ice/Core/Component.h>
#include <Ice/Core/SceneManager.h>

// Constructor
Actor::Actor()
{
	transform = new Transform(this);
	components = new std::vector<Component*>;

	SceneManager::GetInstance().AddActor(this);
}

// Deconstructor
Actor::~Actor()
{
	delete transform;
	for (Component* component : *components)
	{
		delete component;
	}
	delete components;
}

// Add Component by Type
template <typename T>
T* Actor::AddComponent()
{
	T* newComponent = new T;
	newComponent->owner = this;
	newComponent->transform = transform;
	components->push_back(newComponent);
	return newComponent;
}

// Get Component by Type
template <typename T>
T* Actor::GetComponent()
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
void Actor::RemoveComponent(Component* component)
{
	for (int i = 0; i < components->size(); i++)
	{
		if (components->at(i) == component)
		{
			components->erase(components->begin() + i);
			return;
		}
	}
}


// Delete Component by Pointer
void Actor::DeleteComponent(Component* component)
{
	for (int i = 0; i < components->size(); i++)
	{
		if (components->at(i) == component)
		{
			components->erase(components->begin() + i);
			delete component;
			return;
		}
	}
}


// Add Component by Pointer
void Actor::AddComponent(Component* component)
{
	if (component->owner != nullptr)
	{
		component->owner->RemoveComponent(component);
	}
	component->owner = this;
	component->transform = transform;
	components->push_back(component);
}