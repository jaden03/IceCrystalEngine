#pragma once

#ifndef INPUT_H

#define INPUT_H

#include <vector>
#include <GLFW/glfw3.h>

#include <Ice/Core/WindowManager.h>

class Input
{

public:

	static Input& GetInstance()
	{
		static Input instance; // Static local variable ensures a single instance
		return instance;
	}

	// Keyboard Input

	static bool GetKeyDown(int key);
	static bool GetKeyUp(int key);
	static bool GetKey(int key);

	
	// Mouse Input
	


	// Clear Input

	void ClearInput();

private:

	GLFWwindow* window;
	WindowManager& windowManager = WindowManager::GetInstance();

	
	// Storing keys
	
	static std::vector<int> keysPressed;
	static std::vector<int> keysJustPressed;
	static std::vector<int> keysJustReleased;


	// Callback for keyboard input
	
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	
	



	Input(); // Private constructor to ensure a single instance

	Input(Input const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(Input const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working
};

#endif