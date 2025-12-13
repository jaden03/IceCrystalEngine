#pragma once

#ifndef SCENE_MANAGER_H

#define SCENE_MANAGER_H

#include <vector>
#include <string>

#include <glm/glm.hpp>

class Actor;
class Camera;


// This is how to make a singleton class

class SceneManager
{

public:
	
	static SceneManager& GetInstance();
	
	Camera* mainCamera;
	double gameTime = 0.0f;
	float deltaTime = 0.0f;
	double lastFrameTime = 0.0;

	int usedTextureCount = 10; // start at 10 every frame, we reserve the first 10 for things like shadow maps and
	// obv the current rendering texture is always 0
	
	void Update();
	
	int GetActorCount();

	void AddActor(Actor* actor);
	void RemoveActor(Actor* actor);

	Actor* GetHoveredActor() {return hoveredActor;}

	// Returns the first actor with the given tag
	Actor* GetActorByTag(std::string tag);
		
	// Returns all actors with the given tag
	std::vector<Actor*> GetActorsByTag(std::string tag);

	// Returns the first component of the given type
	template <typename T>
	T* GetComponentOfType();


	glm::vec2 uiPosition = glm::vec2(0, 0);
	glm::vec2 uiSize = glm::vec2(50, 50);

	// Get all actors in the scene
	std::vector<Actor*>* GetActors() { return actors; }

private:
	std::vector<Actor*>* actors;
	std::vector<glm::vec3> usedActorColors;

	Actor* hoveredActor;
	
	SceneManager(); // Private constructor to ensure a single instance
	~SceneManager();
	
	SceneManager(SceneManager const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(SceneManager const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif