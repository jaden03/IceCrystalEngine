#include <Ice/Utils/DebugUtil.h>

#include <Ice/Utils/FileUtil.h>

#include <Ice/Core/Actor.h>

#include "glm/gtc/type_ptr.inl"
#include "imgui/imgui_internal.h"

DebugUtil::DebugUtil()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(windowManager.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	font = io.Fonts->AddFontFromFileTTF(FileUtil::SubstituteVariables("{ASSET_DIR}Fonts/Varela.ttf").c_str(), 20.0f);
	fontBig = io.Fonts->AddFontFromFileTTF(FileUtil::SubstituteVariables("{ASSET_DIR}Fonts/Varela.ttf").c_str(), 50.0f);
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.06f, 0.06f, 0.06f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.43f, 0.43f, 0.43f, 0.40f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.86f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
	colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.86f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	//colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
	//colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


	style.WindowMenuButtonPosition = ImGuiDir_None;
	style.WindowRounding = 6.0f;
	style.ChildRounding = 0.0f;
	style.FrameRounding = 6.0f;
	style.PopupRounding = 6.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 6.0f;

	style.WindowBorderSize = 0.0f;
	style.ChildBorderSize = 0.0f;
	
	style.FramePadding = ImVec2(10.0f, 6.0f);

	// Setup the buffer to output to the console
	oldbuf = std::cout.rdbuf(ss.rdbuf());

	// First text in the console
	std::cout << "[ IceCrystalEngine ]\n";
	std::cout << "----------------------------------------\n";

	std::cout << " Rendering Backend\n";
	std::cout << "   OpenGL Version : " << glGetString(GL_VERSION) << "\n";
	std::cout << "   GLSL Version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";

	std::cout << " Debug Console Initializing\n";
	SetupCommands();
	std::cout << " Debug Console Initialized\n";
	std::cout << "  Toggle with: ~\n";
	std::cout << "----------------------------------------\n";
}


void DebugUtil::StartOfFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Calculate FPS and store it in the fpsHistory buffer
	float currentFPS = 1.0f / sceneManager.deltaTime;
	fpsHistory.push_back(currentFPS);

	// Remove the oldest FPS entry if the buffer exceeds the max history size
	if (fpsHistory.size() > static_cast<size_t>(fpsHistorySize)) {
		fpsHistory.pop_front();
	}
}

void DebugUtil::EndOfFrame()
{
	ImGui::PushFont(font);

	ImGui::SetNextWindowSize(ImVec2(300, 300));
	ImGui::SetNextWindowPos(ImVec2(50, 50));
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	
	ImGui::SetWindowFontScale(1.25f);
	ImGui::Text("Debug Menu");
	ImGui::SetWindowFontScale(1);
	ImGui::Separator();
	
	ImGui::Checkbox("Show Console", &showConsole);
	ImGui::Separator();

	
	// Calculate the average FPS from fpsHistory for a smoother display
	float avgFPS = 0.0f;
	for (float fps : fpsHistory)
		avgFPS += fps;
	avgFPS /= fpsHistory.size();
	ImGui::Text("FPS: %i", static_cast<int>(round(avgFPS)));
	
	ImGui::Text("Actors: %i", sceneManager.GetActorCount());
	

	if (sceneManager.GetHoveredActor() != nullptr)
		ImGui::Text("Hovered Actor: %s", sceneManager.GetHoveredActor()->name.c_str());
	else
		ImGui::Text("Hovered Actor: None");

	
	ImGui::Separator();
	
	ImGui::Text("Directional Lights: %i", lightingManager.directionalLights.size());
	ImGui::Text("Point Lights: %i", lightingManager.pointLights.size());
	ImGui::Text("Spot Lights: %i", lightingManager.spotLights.size());


	// Testing Stuff
	ImGui::DragFloat2("UI Position", glm::value_ptr(sceneManager.uiPosition), .5f);
	ImGui::DragFloat2("UI Size", glm::value_ptr(sceneManager.uiSize), .5f);
	
	
	ImGui::End();


	if (showConsole)
	{
		// Set the initial size for the console window, but allow the user to resize it.
		ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);

		// Begin a window with title and resizing options enabled
		ImGui::Begin("Console", &showConsole, ImGuiWindowFlags_None);

		// Display console content
		std::string ssText = ss.str();

		// Start a child region to allow the console content to scroll
		ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing() - 16), true);
		ImGui::TextWrapped("%s", ssText.c_str());

		// Auto-scroll to the bottom
		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
		ImGui::EndChild();

		// Input buffer for user input
		static char inputBuffer[256] = "";
		// Stretch the input box across the entire width
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

		// Input box for commands or messages
		if (ImGui::InputText("##ConsoleInput", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			// Handle input when the user presses Enter
			std::string inputText(inputBuffer);

			RunDebugCommand(inputText);

			ss << "> " << inputText << "\n";  // Echo the input back to the console
			// Clear the input buffer
			inputBuffer[0] = '\0';

			// Set focus back on input box
			ImGui::SetKeyboardFocusHere(-1);
		}

		ImGui::PopItemWidth();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
		{
			ImGui::ClearActiveID();
		}

		ImGui::End();

	}
	

	ImGui::PopFont();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugUtil::Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}


void DebugUtil::RegisterCommand(const std::string& commandName, const CommandHandler& handler)
{
	std::cout << "   Registered command: " << commandName << "\n";
	commandMap[commandName] = handler;
}

void DebugUtil::RunDebugCommand(const std::string& command)
{
	std::istringstream iss(command);
	std::vector<std::string> tokens;
	std::string token;

	// Split command by spaces
	while (iss >> token)
	{
		tokens.push_back(token);
	}

	if (tokens.empty())
	{
		std::cout << "No command entered!\n";
		return;
	}

	// Find and execute the command handler
	auto it = commandMap.find(tokens[0]);
	if (it != commandMap.end())
	{
		it->second(tokens); // call the handler with the tokens
	}
	else
	{
		std::cout << "Unknown command entered: " << tokens[0] << "\n";
	}
}