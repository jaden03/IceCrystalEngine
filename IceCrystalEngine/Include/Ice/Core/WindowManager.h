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

	bool isFullscreen = false;

	int GetWindowWidth() const { return windowWidth; }
	int GetWindowHeight() const { return windowHeight; }

	void ToggleFullscreen();
	void SetFullscreen(bool fullscreen);

private:

	WindowManager(); // Private constructor to ensure a single instance

	WindowManager(WindowManager const&) = delete; // Delete copy constructor
	void operator=(WindowManager const&) = delete; // Delete assignment operator

	void InitializeWindow();

	// Store windowed mode state
	int windowedWidth = 1920;
	int windowedHeight = 1080;
	int windowedX = 100;
	int windowedY = 100;
};

#endif