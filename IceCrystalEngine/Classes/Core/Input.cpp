#include <Ice/Core/Input.h>

#include <iostream>

WindowManager& Input::windowManager = WindowManager::GetInstance();
GLFWwindow* Input::window = windowManager.window;

std::vector<int> Input::keysPressed;
std::vector<int> Input::keysJustPressed;
std::vector<int> Input::keysJustReleased;

std::vector<int> Input::mouseButtonsPressed;
std::vector<int> Input::mouseButtonsJustPressed;
std::vector<int> Input::mouseButtonsJustReleased;

std::vector<InputAxis> Input::inputAxes;

bool Input::scrolledUp = false;
bool Input::scrolledDown = false;

bool Input::lockCursor = false;
bool Input::hideCursor = false;

Input::Input()
{
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}


// Clear Input (called in the main loop)
void Input::ClearInput()
{
	if (lockCursor)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		if (hideCursor)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	keysJustPressed.clear();
	keysJustReleased.clear();
	mouseButtonsJustPressed.clear();
	mouseButtonsJustReleased.clear();
	scrolledUp = false;
	scrolledDown = false;
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



// Callback for mouse input

void Input::mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		mouseButtonsPressed.push_back(button);
		mouseButtonsJustPressed.push_back(button);
	}
	else if (action == GLFW_RELEASE)
	{
		mouseButtonsPressed.erase(std::remove(mouseButtonsPressed.begin(), mouseButtonsPressed.end(), button), mouseButtonsPressed.end());
		mouseButtonsJustReleased.push_back(button);
	}
}



// Mouse Input

bool Input::GetMouseButtonDown(int button)
{
	for (int i = 0; i < mouseButtonsJustPressed.size(); i++)
	{
		if (mouseButtonsJustPressed[i] == button)
		{
			return true;
		}
	}

	return false;
}

bool Input::GetMouseButtonUp(int button)
{
	for (int i = 0; i < mouseButtonsJustReleased.size(); i++)
	{
		if (mouseButtonsJustReleased[i] == button)
		{
			return true;
		}
	}

	return false;
}

bool Input::GetMouseButton(int button)
{
	for (int i = 0; i < mouseButtonsPressed.size(); i++)
	{
		if (mouseButtonsPressed[i] == button)
		{
			return true;
		}
	}

	return false;
}


// Callback for scrollwheel

void Input::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset == -1)
		scrolledDown = true;
	else if (yoffset == 1)
		scrolledUp = true;
}



// Input Axes

InputAxis::InputAxis(std::string name, int positiveKey, int negativeKey)
{
	this->name = name;
	this->positiveKey = positiveKey;
	this->negativeKey = negativeKey;
}


void Input::CreateAxis(std::string name, int positiveKey, int negativeKey)
{
	InputAxis axis = InputAxis(name, positiveKey, negativeKey);
	inputAxes.push_back(axis);
}

int Input::GetAxis(std::string name)
{
	for (int i = 0; i < inputAxes.size(); i++)
	{
		if (inputAxes[i].name == name)
		{
			int value = 0;

			if (GetKey(inputAxes[i].positiveKey))
				value += 1;
			if (GetKey(inputAxes[i].negativeKey))
				value -= 1;

			return value;
		}
	}

	return 0;
}



void Input::GetMousePosition(double* x, double* y)
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	*x = xpos;
	*y = ypos;
}

glm::vec2 Input::GetMousePosition()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	return glm::vec2(xpos, ypos);
}