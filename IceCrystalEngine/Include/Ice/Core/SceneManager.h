#pragma once

#ifndef SCENE_MANAGER_H

#define SCENE_MANAGER_H

#include <vector>
#include <string>

class Actor;
class Camera;


// This is how to make a singleton class

class SceneManager
{

public:
	
	static SceneManager& GetInstance()
	{
		static SceneManager instance; // Static local variable ensures a single instance
		return instance;
	}
	
	Camera* mainCamera;
	float deltaTime;
	
	void Update();
	void AddActor(Actor* actor);

	// Returns the first actor with the given tag
	Actor* GetActorByTag(std::string tag);
		
	// Returns all actors with the given tag
	std::vector<Actor*> GetActorsByTag(std::string tag);

	// Returns the first component of the given type
	template <typename T>
	T* GetComponentOfType();
	

private:


	std::vector<Actor*>* actors;
	
	SceneManager(); // Private constructor to ensure a single instance
	~SceneManager();
	
	SceneManager(SceneManager const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(SceneManager const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif