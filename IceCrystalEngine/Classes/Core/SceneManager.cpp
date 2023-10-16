#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Component.h>

// Constructor
SceneManager::SceneManager()
{
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
	// loop through actors
	for (int i = 0; i < actors->size(); i++)
	{
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