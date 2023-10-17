#pragma once

#ifndef SCENE_INITIALIZER_H

#define SCENE_INITIALIZER_H

class SceneInitializer
{

public:

	static SceneInitializer& GetInstance()
	{
		static SceneInitializer instance; // Static local variable ensures a single instance
		return instance;
	}

	void InitializeScene();

private:

	SceneInitializer(); // Private constructor to ensure a single instance
	//~SceneInitializer();

	SceneInitializer(SceneInitializer const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(SceneInitializer const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working

};

#endif