#pragma once

#ifndef DEBUG_UTIL_H

#define DEBUG_UTIL_H

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/SceneManager.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <iostream>
#include <sstream>

class DebugUtil
{

	WindowManager& windowManager = WindowManager::GetInstance();
	SceneManager& sceneManager = SceneManager::GetInstance();

	DebugUtil();
	DebugUtil(DebugUtil const&) = delete;
	void operator=(DebugUtil const&) = delete;

	
	std::stringstream ss;
	std::streambuf* oldbuf;

	std::string currentConsoleText;

	ImFont* font;
	ImFont* fontBig;

	
public:
	
	static DebugUtil& GetInstance()
	{
		static DebugUtil instance; // Static local variable ensures a single instance
		return instance;
	}

	void StartOfFrame();
	void EndOfFrame();

	void Cleanup();


	bool showConsole = true;

};

#endif