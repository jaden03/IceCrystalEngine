#pragma once

#ifndef WINDOW_MANAGER_H

#define WINDOW_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class WindowManager
{
	
public:

	static WindowManager& GetInstance()
	{
		static WindowManager instance; // Static local variable ensures a single instance
		return instance;
	}

	GLFWwindow* window;

	int windowWidth = 1920;
	int windowHeight = 1080;

	// this cannot be changed at runtime (yet)
	// also this is borderless, because fullscreen is trash
	bool isFullscreen = false;

	int GetWindowWidth() const { return windowWidth; }
	int GetWindowHeight() const { return windowHeight; }

private:

	WindowManager(); // Private constructor to ensure a single instance

	WindowManager(WindowManager const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(WindowManager const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working


	void InitializeWindow();
};

#endif