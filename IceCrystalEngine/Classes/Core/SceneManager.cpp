#include <iostream>

#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Component.h>
#include <Ice/Core/Transform.h>

// Constructor
SceneManager::SceneManager()
{
	mainCamera = nullptr;
	deltaTime = 0.0f;
	actors = new std::vector<Actor*>();
}

// Deconstructor
SceneManager::~SceneManager()
{
	for (int i = 0; i < actors->size(); i++)
	{
		delete actors->at(i);
	}
	delete actors;
}

// Update
void SceneManager::Update()
{
	// get the mainCamera
	if (mainCamera == nullptr)
	{
		mainCamera = GetComponentOfType<Camera>();

		if (mainCamera == nullptr)
		{
			// create a fallback camera
			Actor* cameraActor = new Actor("Main Camera", "MainCamera");
			cameraActor->AddComponent<Camera>();

			// TODO : add proper logging
			std::cout << "No Camera Component found in Scene, A fallback Actor with a Camera Component has been created. You can access the Actor by the tag \"MainCamera\"." << std::endl;
		}
	}

	// loop through actors
	for (int i = 0; i < actors->size(); i++)
	{
		// update the transform
		actors->at(i)->transform->Update();
		// loop through components
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			actors->at(i)->components->at(j)->Update();
		}
	}
}

// Add Actor
void SceneManager::AddActor(Actor* actor)
{
	actors->push_back(actor);
}


// Get Actor by Tag
Actor* SceneManager::GetActorByTag(std::string tag)
{
	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->tag == tag)
		{
			return actors->at(i);
		}
	}
	return nullptr;
}

// Get Actors by Tag
std::vector<Actor*> SceneManager::GetActorsByTag(std::string tag)
{
	std::vector<Actor*> actorsWithTag = std::vector<Actor*>();

	for (int i = 0; i < actors->size(); i++)
	{
		if (actors->at(i)->tag == tag)
		{
			actorsWithTag.push_back(actors->at(i));
		}
	}

	return actorsWithTag;
}



// Get Component of Type
template <typename T>
T* SceneManager::GetComponentOfType()
{
	for (int i = 0; i < actors->size(); i++)
	{
		for (int j = 0; j < actors->at(i)->components->size(); j++)
		{
			if (dynamic_cast<T*>(actors->at(i)->components->at(j)) != nullptr)
			{
				return dynamic_cast<T*>(actors->at(i)->components->at(j));
			}
		}
	}
	return nullptr;
}