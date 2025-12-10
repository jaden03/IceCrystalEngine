#include <Ice/Editor/EditorUI.h>
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/WindowManager.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/Input.h>
#include <Ice/Editor/GizmoRenderer.h>
#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h>
#include <Ice/Components/Camera.h>
#include <Ice/Components/Light.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Physics/RigidBody.h>
#include <Ice/Utils/FileUtil.h>

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

// Constructor
EditorUI::EditorUI()
    : editorEnabled(true)
    , enginePaused(false)
    , selectedActor(nullptr)
    , defaultFont(nullptr)
    , boldFont(nullptr)
    , titleFont(nullptr)
    , showDemoWindow(false)
    , hierarchyWidth(300.0f)
    , inspectorWidth(350.0f)
    , toolbarHeight(50.0f)
    , consoleAutoScroll(true)
    , currentGizmoMode(0)
    , gizmosEnabled(true)
    , showCreateActorPopup(false)
    , deltaTime(0.0f)
    , fps(0.0f)
{
    memset(consoleInputBuffer, 0, sizeof(consoleInputBuffer));
    memset(newActorNameBuffer, 0, sizeof(newActorNameBuffer));
    strcpy_s(newActorNameBuffer, "New Actor");

    accentColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    highlightColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
    backgroundColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    windowBgColor = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    
    // Initialize ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Docking support may not be available in this ImGui version
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    
    // Initialize ImGui for GLFW and OpenGL3
    WindowManager& windowManager = WindowManager::GetInstance();
    ImGui_ImplGlfw_InitForOpenGL(windowManager.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// Destructor
EditorUI::~EditorUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Initialize
void EditorUI::Initialize()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Load fonts
    std::string fontPath = FileUtil::SubstituteVariables("{ASSET_DIR}Fonts/Varela.ttf");
    if (!fontPath.empty())
    {
        defaultFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
        boldFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 18.0f);
        titleFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);
    }
    
    SetupEditorStyle();
    
    AddConsoleLog("Ice Crystal Engine Editor Initialized", glm::vec4(0.4f, 0.8f, 0.4f, 1.0f));
}

// Cleanup
void EditorUI::Cleanup()
{
    consoleMessages.clear();
}

// Setup editor style
void EditorUI::SetupEditorStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Rounding
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    // Padding and spacing
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(8.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    
    // Colors - Dark theme with blue accents
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = windowBgColor;
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.3f, 0.3f, 0.3f, 0.5f);
    
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
    
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
    
    colors[ImGuiCol_Header] = accentColor;
    colors[ImGuiCol_HeaderHovered] = highlightColor;
    colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.75f, 1.0f, 1.0f);
    
    colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.0f);
    
    colors[ImGuiCol_CheckMark] = accentColor;
    colors[ImGuiCol_SliderGrab] = accentColor;
    colors[ImGuiCol_SliderGrabActive] = highlightColor;
}

// Begin frame
void EditorUI::BeginFrame()
{
    if (!editorEnabled) return;
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Update performance metrics
    SceneManager& sceneManager = SceneManager::GetInstance();
    deltaTime = sceneManager.deltaTime;
    fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    
    fpsHistory.push_back(fps);
    if (fpsHistory.size() > fpsHistorySize)
    {
        fpsHistory.erase(fpsHistory.begin());
    }
}

// Render editor
void EditorUI::RenderEditor()
{
    if (!editorEnabled) return;
    
    RenderMainMenuBar();
    
    if (panelVisibility.toolbar)
        RenderToolbar();
    
    if (panelVisibility.hierarchy)
        RenderHierarchy();
    
    if (panelVisibility.inspector)
        RenderInspector();
    
    if (panelVisibility.sceneStats)
        RenderSceneStats();
    
    if (panelVisibility.console)
        RenderConsole();
    
    // ImGui demo for reference (toggle with F1)
    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);
}

// End frame
void EditorUI::EndFrame()
{
    if (!editorEnabled) return;
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Render main menu bar
void EditorUI::RenderMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) 
            {
                AddConsoleLog("New Scene (not implemented)", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) 
            {
                AddConsoleLog("Save Scene (not implemented)", glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                WindowManager& wm = WindowManager::GetInstance();
                glfwSetWindowShouldClose(wm.window, true);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("GameObject"))
        {
            if (ImGui::MenuItem("Create Empty Actor"))
            {
                showCreateActorPopup = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("3D Object > Cube", nullptr, false, false)) {}
            if (ImGui::MenuItem("3D Object > Sphere", nullptr, false, false)) {}
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Hierarchy", nullptr, &panelVisibility.hierarchy);
            ImGui::MenuItem("Inspector", nullptr, &panelVisibility.inspector);
            ImGui::MenuItem("Scene Stats", nullptr, &panelVisibility.sceneStats);
            ImGui::MenuItem("Console", nullptr, &panelVisibility.console);
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", "F1", &showDemoWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Documentation")) 
            {
                AddConsoleLog("Opening documentation...", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
            }
            if (ImGui::MenuItem("About"))
            {
                AddConsoleLog("Ice Crystal Engine v1.0", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Create Actor Popup
    if (showCreateActorPopup)
    {
        ImGui::OpenPopup("Create Actor");
        showCreateActorPopup = false;
    }
    
    if (ImGui::BeginPopupModal("Create Actor", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Enter actor name:");
        ImGui::InputText("##ActorName", newActorNameBuffer, sizeof(newActorNameBuffer));
        
        ImGui::Separator();
        
        if (ImGui::Button("Create", ImVec2(120, 0)))
        {
            Actor* newActor = new Actor(newActorNameBuffer);
            SceneManager::GetInstance().AddActor(newActor);
            AddConsoleLog("Created actor: " + std::string(newActorNameBuffer), glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

// Render toolbar
void EditorUI::RenderToolbar()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2((float)windowManager.windowWidth, toolbarHeight));
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
    
    if (ImGui::Begin("Toolbar", nullptr, flags))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
        
        // Play/Pause controls
        ImGui::SameLine(20);
        
        if (enginePaused)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button("Play", ImVec2(80, 30)))
            {
                enginePaused = false;
                AddConsoleLog("Engine resumed", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
            if (ImGui::Button("Pause", ImVec2(80, 30)))
            {
                enginePaused = true;
                AddConsoleLog("Engine paused", glm::vec4(1.0f, 1.0f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
        }
        
        // Separator
        ImGui::SameLine(120);
        ImGui::Text("|"); // Simple vertical separator
        
        // Gizmo controls
        ImGui::SameLine(140);
        ImGui::Text("Gizmo:");
        
        ImGui::SameLine();
        GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
        
        ImGui::SameLine();
        bool wasEnabled = gizmoRenderer.IsEnabled();
        if (wasEnabled)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        if (ImGui::Button(wasEnabled ? "Hide Gizmos (G)" : "Show Gizmos (G)", ImVec2(115, 30)))
        {
            gizmoRenderer.SetEnabled(!wasEnabled);
        }
        if (wasEnabled)
            ImGui::PopStyleColor();
        
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

// Render hierarchy panel
void EditorUI::RenderHierarchy()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    SceneManager& sceneManager = SceneManager::GetInstance();
    
    ImGui::SetNextWindowPos(ImVec2(0, 70), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(hierarchyWidth, (float)windowManager.windowHeight - 320), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Hierarchy", &panelVisibility.hierarchy))
    {
        hierarchyWidth = ImGui::GetWindowWidth();
        
        ImGui::Text("Scene Actors (%d)", sceneManager.GetActorCount());
        ImGui::Separator();
        
        if (ImGui::Button("+ Create Actor", ImVec2(-1, 0)))
        {
            showCreateActorPopup = true;
        }
        
        ImGui::Separator();
        
        // List all actors
        std::vector<Actor*>* actors = sceneManager.GetActors();
        for (int i = 0; i < actors->size(); i++)
        {
            RenderActorNode(actors->at(i), i);
        }
    }
    ImGui::End();
}

// Render actor node in hierarchy
void EditorUI::RenderActorNode(Actor* actor, int index)
{
    if (!actor) return;
    
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    
    if (actor == selectedActor)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    
    ImGui::TreeNodeEx((void*)(intptr_t)index, flags, "%s", actor->name.c_str());
    
    if (ImGui::IsItemClicked())
    {
        HandleActorSelection(actor);
    }
    
    // Context menu
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete"))
        {
            SceneManager::GetInstance().RemoveActor(actor);
            if (selectedActor == actor)
                selectedActor = nullptr;
            AddConsoleLog("Deleted actor: " + actor->name, glm::vec4(1.0f, 0.6f, 0.4f, 1.0f));
        }
        if (ImGui::MenuItem("Duplicate", nullptr, false, false))
        {
            // Not implemented
        }
        ImGui::EndPopup();
    }
}

// Handle actor selection
void EditorUI::HandleActorSelection(Actor* actor)
{
    selectedActor = actor;
    AddConsoleLog("Selected: " + actor->name, glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
}

// Render inspector panel
void EditorUI::RenderInspector()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    
    float xPos = (float)windowManager.windowWidth - inspectorWidth;
    ImGui::SetNextWindowPos(ImVec2(xPos, 70), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(inspectorWidth, (float)windowManager.windowHeight - 320), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Inspector", &panelVisibility.inspector))
    {
        inspectorWidth = ImGui::GetWindowWidth();
        
        if (selectedActor)
        {
            // Actor name
            ImGui::PushFont(boldFont ? boldFont : defaultFont);
            ImGui::Text("%s", selectedActor->name.c_str());
            ImGui::PopFont();
            
            ImGui::Separator();
            
            // Transform
            if (selectedActor->transform)
            {
                RenderTransformInspector(selectedActor->transform);
            }
            
            ImGui::Separator();
            
            // Components
            ImGui::Text("Components:");
            
            // Iterate through components vector (it's a pointer to vector)
            if (selectedActor->components)
            {
                for (size_t i = 0; i < selectedActor->components->size(); i++)
                {
                    Component* component = selectedActor->components->at(i);
                    if (component)
                    {
                        RenderComponentInspector(component);
                    }
                }
            }
            
            ImGui::Separator();
            
            // Add component button
            if (ImGui::Button("+ Add Component", ImVec2(-1, 0)))
            {
                ImGui::OpenPopup("AddComponent");
            }
            
            RenderAddComponentMenu();
        }
        else
        {
            ImGui::TextWrapped("No actor selected. Select an actor from the Hierarchy panel.");
        }
    }
    ImGui::End();
}

// Render transform inspector
void EditorUI::RenderTransformInspector(Transform* transform)
{
    if (!transform) return;
    
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
    {
        DrawVec3Control("Position", transform->position);
        DrawVec3Control("Rotation", transform->eulerAngles);
        DrawVec3Control("Scale", transform->scale, 1.0f);
    }
}

// Render component inspector
void EditorUI::RenderComponentInspector(Component* component)
{
    if (!component) return;
    
    std::string typeName = GetComponentTypeName(component);
    std::string headerLabel = typeName + "##" + std::to_string((intptr_t)component);
    
    if (ImGui::CollapsingHeader(headerLabel.c_str()))
    {
        ImGui::Indent();
        
        // For now, just show component type - property editing can be added later
        ImGui::TextDisabled("(Component properties - edit via code)");
        
        // You can add specific property editors here as needed
        // Example: if (Camera* camera = dynamic_cast<Camera*>(component)) { ... }
        
        ImGui::Unindent();
    }
}

// Render add component menu
void EditorUI::RenderAddComponentMenu()
{
    if (ImGui::BeginPopup("AddComponent"))
    {
        // Component adding can be extended here
        ImGui::TextDisabled("Component adding coming soon...");
        
        // Example: 
        // if (ImGui::MenuItem("Camera")) { selectedActor->AddComponent<Camera>(); }
        ImGui::EndPopup();
    }
}

// Render scene stats panel
void EditorUI::RenderSceneStats()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    SceneManager& sceneManager = SceneManager::GetInstance();
    LightingManager& lightingManager = LightingManager::GetInstance();
    
    ImGui::SetNextWindowPos(ImVec2(0, (float)windowManager.windowHeight - 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(hierarchyWidth, 250), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Scene Stats", &panelVisibility.sceneStats))
    {
        ImGui::Text("Performance");
        ImGui::Separator();
        
        ImGui::Text("FPS: %.0f", fps);
        ImGui::Text("Frame Time: %.2f ms", deltaTime * 1000.0f);
        
        // FPS Graph
        if (fpsHistory.size() > 1)
        {
            ImGui::PlotLines("##FPS", fpsHistory.data(), (int)fpsHistory.size(), 0, nullptr, 0.0f, 120.0f, ImVec2(0, 60));
        }
        
        ImGui::Separator();
        ImGui::Text("Scene");
        ImGui::Separator();
        
        ImGui::Text("Actors: %d", sceneManager.GetActorCount());
        ImGui::Text("Main Camera: %s", sceneManager.mainCamera ? "Yes" : "No");
        
        Actor* hoveredActor = sceneManager.GetHoveredActor();
        if (hoveredActor)
            ImGui::Text("Hovered: %s", hoveredActor->name.c_str());
        else
            ImGui::Text("Hovered: None");
        
        ImGui::Separator();
        ImGui::Text("Lighting");
        ImGui::Separator();
        
        ImGui::Text("Directional Light: %s", lightingManager.directionalLight ? "Yes" : "No");
        ImGui::Text("Point Lights: %zu", lightingManager.pointLights.size());
        ImGui::Text("Spot Lights: %zu", lightingManager.spotLights.size());
    }
    ImGui::End();
}

// Render console panel
void EditorUI::RenderConsole()
{
    WindowManager& windowManager = WindowManager::GetInstance();
    
    float xPos = (float)windowManager.windowWidth - inspectorWidth;
    ImGui::SetNextWindowPos(ImVec2(xPos, (float)windowManager.windowHeight - 250), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(inspectorWidth, 250), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Console", &panelVisibility.console))
    {
        // Clear button
        if (ImGui::Button("Clear"))
        {
            ClearConsole();
        }
        
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &consoleAutoScroll);
        
        ImGui::Separator();
        
        // Console output
        ImGui::BeginChild("ConsoleOutput", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
        
        for (const auto& msg : consoleMessages)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(msg.color.r, msg.color.g, msg.color.b, msg.color.a));
            ImGui::TextWrapped("%s", msg.message.c_str());
            ImGui::PopStyleColor();
        }
        
        if (consoleAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        
        ImGui::EndChild();
        
        // Console input
        if (ImGui::InputText("##ConsoleInput", consoleInputBuffer, sizeof(consoleInputBuffer), 
                            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (strlen(consoleInputBuffer) > 0)
            {
                AddConsoleLog("> " + std::string(consoleInputBuffer), glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
                memset(consoleInputBuffer, 0, sizeof(consoleInputBuffer));
            }
            ImGui::SetKeyboardFocusHere(-1);
        }
    }
    ImGui::End();
}

// Get component type name
std::string EditorUI::GetComponentTypeName(Component* component)
{
    if (!component) return "Unknown";
    
    if (dynamic_cast<Camera*>(component)) return "Camera";
    if (dynamic_cast<Renderer*>(component)) return "Renderer";
    if (dynamic_cast<RigidBody*>(component)) return "RigidBody";
    
    return "Component";
}

// Draw Vec3 control
void EditorUI::DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
{
    ImGui::PushID(label.c_str());
    
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, columnWidth);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    
    float lineHeight = ImGui::GetTextLineHeight();
    ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };
    
    float widthEach = (ImGui::CalcItemWidth() - buttonSize.x * 3) / 3.0f;
    
    // X
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
    if (ImGui::Button("X", buttonSize))
        values.x = resetValue;
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(widthEach);
    ImGui::DragFloat("##X", &values.x, 0.1f);
    ImGui::SameLine();
    
    // Y
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    if (ImGui::Button("Y", buttonSize))
        values.y = resetValue;
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(widthEach);
    ImGui::DragFloat("##Y", &values.y, 0.1f);
    ImGui::SameLine();
    
    // Z
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
    if (ImGui::Button("Z", buttonSize))
        values.z = resetValue;
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(widthEach);
    ImGui::DragFloat("##Z", &values.z, 0.1f);
    
    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();
}

// Draw float control
void EditorUI::DrawFloatControl(const std::string& label, float& value, float resetValue, float speed)
{
    ImGui::PushID(label.c_str());
    
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    
    ImGui::DragFloat("##Value", &value, speed);
    
    ImGui::Columns(1);
    ImGui::PopID();
}

// Draw color control
void EditorUI::DrawColorControl(const std::string& label, glm::vec3& color)
{
    ImGui::PushID(label.c_str());
    
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 100.0f);
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();
    
    ImGui::ColorEdit3("##Color", glm::value_ptr(color));
    
    ImGui::Columns(1);
    ImGui::PopID();
}

// Add console log
void EditorUI::AddConsoleLog(const std::string& message, const glm::vec4& color)
{
    ConsoleMessage msg;
    msg.message = message;
    msg.color = color;
    msg.timestamp = (float)glfwGetTime();
    
    consoleMessages.push_back(msg);
    
    // Limit console history
    if (consoleMessages.size() > 500)
    {
        consoleMessages.erase(consoleMessages.begin());
    }
}

// Clear console
void EditorUI::ClearConsole()
{
    consoleMessages.clear();
}