#pragma once

#ifndef INPUT_H

#define INPUT_H

#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Ice/Managers/WindowManager.h>


class InputAxis
{
public:

	std::string name;
	int positiveKey;
	int negativeKey;

	InputAxis(std::string name, int positiveKey, int negativeKey);

};


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
	
	static bool GetMouseButtonDown(int button);
	static bool GetMouseButtonUp(int button);
	static bool GetMouseButton(int button);


	// Mouse Position
	static void GetMousePosition(double* mouseX, double* mouseY);
	static glm::vec2 GetMousePosition();
	
	// Scroll Input
	static bool scrolledUp;
	static bool scrolledDown;


	// Input Axes
	
	static void CreateAxis(std::string name, int positiveKey, int negativeKey);
	static int GetAxis(std::string axisName);


	// Cursor

	static bool lockCursor;
	static bool hideCursor;


	// Clear Input

	void ClearInput();

private:


	static WindowManager& windowManager;;
	static GLFWwindow* window;
	
	// Storing keys
	
	static std::vector<int> keysPressed;
	static std::vector<int> keysJustPressed;
	static std::vector<int> keysJustReleased;

	static std::vector<int> mouseButtonsPressed;
	static std::vector<int> mouseButtonsJustPressed;
	static std::vector<int> mouseButtonsJustReleased;

	static std::vector<InputAxis> inputAxes;

	// Callback for keyboard input
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	
	// Callback for mouse input
	static void mouse_callback(GLFWwindow* window, int button, int action, int mods);

	// Callback for scrollwheel
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	

	Input(); // Private constructor to ensure a single instance

	Input(Input const&) = delete; // Delete copy constructor
	// this prevents the copy constructor "SceneManager copy(original);" from working

	void operator=(Input const&) = delete; // Delete assignment operator
	// this prevents copying by assignment "SceneManager another = original;" from working
};

#endif