#include <Ice/Utils/DebugUtil.h>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

DebugUtil::DebugUtil()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(windowManager.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	oldbuf = std::cout.rdbuf(ss.rdbuf());
	std::cout << "The last message in the console will be displayed here, press ""~"" to toggle this console." << std::endl;
}


void DebugUtil::StartOfFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void DebugUtil::EndOfFrame()
{
	ImGui::SetNextWindowPos(ImVec2(50, 50));
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::Text("FPS: %f", 1.0f / sceneManager.deltaTime);
	ImGui::End();


	if (showConsole)
	{
		ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 135));
		ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 135));
		ImGui::Begin("Console", &showConsole, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		float consoleWidthFraction = 0.9;
		ImGui::SetWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 135), ImGuiCond_Always);
		ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * consoleWidthFraction, 0), ImGuiCond_Always);

		std::string ssText = ss.str();

		std::vector<std::string> lines;
		std::istringstream ssStream(ssText);
		std::string line;
		while (std::getline(ssStream, line)) {
			lines.push_back(line);
		}
		std::reverse(lines.begin(), lines.end());

		const int maxLines = 7;
		for (int i = 0; i < std::min(static_cast<int>(lines.size()), maxLines); ++i) {
			ImGui::Text("%s", lines[i].c_str());
		}
		ImGui::End();
	}



	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugUtil::Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}