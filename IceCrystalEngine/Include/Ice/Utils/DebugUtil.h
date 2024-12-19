#pragma once

#ifndef DEBUG_UTIL_H

#define DEBUG_UTIL_H

#include <functional>
#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/LightingManager.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <iostream>
#include <sstream>
#include <deque>

#include "glm/gtx/compatibility.hpp"

class DebugUtil
{
	using CommandHandler = std::function<void(const std::vector<std::string>&)>;

	WindowManager& windowManager = WindowManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();
	LightingManager& lightingManager = LightingManager::GetInstance();

	DebugUtil();
	DebugUtil(DebugUtil const&) = delete;
	void operator=(DebugUtil const&) = delete;


	std::streambuf* oldbuf;

	std::string currentConsoleText;

	ImFont* font;
	ImFont* fontBig;


	std::deque<float> fpsHistory;
	const int fpsHistorySize = 60;

	
	
	std::unordered_map<std::string, CommandHandler> commandMap;

	void SetupCommands()
	{
		RegisterCommand("exit", ExitCommand);
		RegisterCommand("close", CloseCommand);
		RegisterCommand("wireframe", WireframeCommand);
	}

	static void ExitCommand(const std::vector<std::string>& args)
	{
		glfwSetWindowShouldClose(WindowManager::GetInstance().window, true);
	}

	static void CloseCommand(const std::vector<std::string>& args)
	{
		GetInstance().showConsole = false;
	}
	
	static void WireframeCommand(const std::vector<std::string>& args)
	{
		// if (args.size() != 2 || args[1] != "on" && args[1] != "off")
		// {
		// 	std::cout << "Error: wireframe on/off" << "\n";
		// 	return;
		// }

		// GetInstance().wireframeMode = args[1] == "on";

		GetInstance().wireframeMode = !GetInstance().wireframeMode;
		glPolygonMode(GL_FRONT_AND_BACK, GetInstance().wireframeMode ? GL_LINE : GL_FILL);
	}
	
public:

	std::stringstream ss;
	
	static DebugUtil& GetInstance()
	{
		static DebugUtil instance; // Static local variable ensures a single instance
		return instance;
	}

	void StartOfFrame();
	void EndOfFrame();

	void Cleanup();
	
	void RegisterCommand(const std::string& commandName, const CommandHandler& handler);
	void RunDebugCommand(const std::string& command);
	
	bool showConsole = true;
	bool wireframeMode = false;

};

#endif