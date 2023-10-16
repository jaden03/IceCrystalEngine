#include <Ice/Core/SceneManager.h>
#include <Ice/Core/Component.h>

SceneManager::SceneManager()
{
	actors = new std::vector<Actor*>();
}

SceneManager::~SceneManager()
{
	for (int i = 0; i < actors->size(); i++)
	{
		delete actors->at(i);
	}
	delete actors;
}

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

void SceneManager::AddActor(Actor* actor)
{
	actors->push_back(actor);
}