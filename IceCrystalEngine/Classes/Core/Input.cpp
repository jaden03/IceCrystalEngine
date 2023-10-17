#include <iostream>

#include <Ice/Core/Input.h>


std::vector<int> Input::keysPressed;
std::vector<int> Input::keysJustPressed;
std::vector<int> Input::keysJustReleased;

Input::Input()
{
	window = windowManager.window;

	glfwSetKeyCallback(window, key_callback);
}


// Clear Input (called in the main loop)
void Input::ClearInput()
{
	keysJustPressed.clear();
	keysJustReleased.clear();
}

// Callback for keyboard input
void Input::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (action == GLFW_PRESS)
	{
		keysPressed.push_back(key);
		keysJustPressed.push_back(key);
	}
	else if (action == GLFW_RELEASE)
	{
		keysPressed.erase(std::remove(keysPressed.begin(), keysPressed.end(), key), keysPressed.end());
		keysJustReleased.push_back(key);
	}
}



// Keyboard Input

bool Input::GetKeyDown(int key)
{
	for (int i = 0; i < keysJustPressed.size(); i++)
	{
		if (keysJustPressed[i] == key)
		{
			return true;
		}
	}

	return false;
}

bool Input::GetKeyUp(int key)
{
	for (int i = 0; i < keysJustReleased.size(); i++)
	{
		if (keysJustReleased[i] == key)
		{
			return true;
		}
	}

	return false;
}

bool Input::GetKey(int key)
{
	for (int i = 0; i < keysPressed.size(); i++)
	{
		if (keysPressed[i] == key)
		{
			return true;
		}
	}

	return false;
}



// Mouse Input