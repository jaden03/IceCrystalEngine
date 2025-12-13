#ifdef ICE_EDITOR

// Include glad/OpenGL headers first, before anything that might include GLFW
#include <glad/glad.h>

// Ice Editor headers (may include glad/OpenGL)
#include <Ice//Editor/EditorCamera.h>
#include <Ice//Editor/GizmoRenderer.h>
#include <Ice//Editor/EditorUI.h>

// Ice Core headers
#include <Ice/Core/SceneManager.h>
#include <Ice/Core/WindowManager.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Core/Input.h>
#include <Ice/Core/Actor.h>
#include <Ice/Core/Transform.h>

// Ice Components
#include <Ice/Components/Camera.h>
#include <Ice/Components/Light.h>
#include <Ice/Components/Renderer.h>
#include <Ice/Components/Physics/RigidBody.h>

// Ice Utils
#include <Ice/Utils/FileUtil.h>

// External libraries
#include <imgui/imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

// Standard library
#include <iostream>
#include <sstream>
#include <iomanip>

EditorUI& EditorUI::GetInstance()
{
    static EditorUI instance;
    return instance;
}

void EditorUI::SetEnabled(bool enabled) { editorEnabled = enabled; }
bool EditorUI::IsEnabled() const { return editorEnabled; }

void EditorUI::SetPlayMode(PlayMode mode) { playMode = mode; }
PlayMode EditorUI::GetPlayMode() const { return playMode; }
bool EditorUI::IsEditMode() const { return playMode == PlayMode::EDIT; }
bool EditorUI::IsPlayMode() const { return playMode == PlayMode::PLAY || playMode == PlayMode::PAUSED; }
bool EditorUI::IsGamePaused() const { return playMode == PlayMode::PAUSED; }

void EditorUI::SetEnginePaused(bool paused) { playMode = paused ? PlayMode::PAUSED : PlayMode::PLAY; }
bool EditorUI::IsEnginePaused() const { return playMode != PlayMode::PLAY; }

void EditorUI::SetSelectedActor(Actor* actor) { selectedActor = actor; }
Actor* EditorUI::GetSelectedActor() const { return selectedActor; }
bool EditorUI::HasSelectedActor() const { return selectedActor != nullptr; }

unsigned int EditorUI::GetViewportFramebuffer() const { return viewportFBO; }
int EditorUI::GetViewportWidth() const { return viewportWidth; }
int EditorUI::GetViewportHeight() const { return viewportHeight; }
bool EditorUI::IsViewportActive() const { return panelVisibility.viewport && viewportFBO != 0; }

bool EditorUI::IsMouseInViewport() const { return isMouseInViewport; }
glm::vec2 EditorUI::GetViewportMousePos() const { return viewportMousePos; }
glm::vec2 EditorUI::GetViewportSize() const { return glm::vec2(viewportWidth, viewportHeight); }

// Constructor
EditorUI::EditorUI()
    : editorEnabled(true)
    , playMode(PlayMode::EDIT)
    , selectedActor(nullptr)
    , defaultFont(nullptr)
    , boldFont(nullptr)
    , titleFont(nullptr)
    , showDemoWindow(false)
    , hierarchyWidth(300.0f)
    , inspectorWidth(350.0f)
    , toolbarHeight(60.0f)
    , consoleAutoScroll(true)
    , currentGizmoMode(0)
    , gizmosEnabled(true)
    , showCreateActorPopup(false)
    , accentColor(0.2f, 0.6f, 1.0f, 1.0f)
    , highlightColor(0.3f, 0.7f, 1.0f, 1.0f)
    , backgroundColor(0.15f, 0.15f, 0.15f, 1.0f)
    , windowBgColor(0.1f, 0.1f, 0.1f, 0.94f)
    , deltaTime(0.0f)
    , fps(0.0f)
    , isDockingInitialized(false)
    , viewportFBO(0)
    , viewportTexture(0)
    , viewportBrightTexture(0)
    , viewportPickingTexture(0)
    , viewportRBO(0)
    , viewportWidth(1280)
    , viewportHeight(720)
    , isMouseInViewport(false)
    , viewportMousePos(0.0f, 0.0f)
    , viewportPosition(0.0f, 0.0f)
{
    memset(consoleInputBuffer, 0, sizeof(consoleInputBuffer));
    memset(newActorNameBuffer, 0, sizeof(newActorNameBuffer));
    strcpy_s(newActorNameBuffer, "New Actor");

    accentColor = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    highlightColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
    backgroundColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    windowBgColor = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
    
    std::cout << "=== EDITOR EditorUI constructor called ===" << std::endl;
    
    // Initialize ImGui context
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    // Enable docking and viewports
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    
    // Initialize ImGui for GLFW and OpenGL3
    WindowManager& windowManager = WindowManager::GetInstance();
    ImGui_ImplGlfw_InitForOpenGL(windowManager.window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
   // WindowManager::GetInstance().SetFullscreen(true);
}

// Destructor
EditorUI::~EditorUI()
{
    DeleteViewportFramebuffer();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

// Initialize
void EditorUI::Initialize()
{
    ImGuiIO& io = ImGui::GetIO();
    
    // Load fonts
    std::string fontPath = FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Fonts/Varela.ttf");
    if (!fontPath.empty())
    {
        defaultFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
        boldFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 18.0f);
        titleFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);
    }
    
    SetupEditorStyle();
    
    // Initialize viewport framebuffer with default size
    CreateViewportFramebuffer(1280, 720);
    
    // Initialize editor camera
    EditorCamera::GetInstance().Initialize(glm::vec3(0.0f, 5.0f, 10.0f));
    
    AddConsoleLog("Ice Crystal Engine Editor Initialized", glm::vec4(0.4f, 0.8f, 0.4f, 1.0f));
    AddConsoleLog("Viewport framebuffer created (1280x720)", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
    AddConsoleLog("Editor camera initialized", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
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
    
    // Setup dockspace
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    // DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    
    // Setup default layout on first run
    if (!isDockingInitialized)
    {
        SetupDefaultDockingLayout(dockspace_id);
        isDockingInitialized = true;
    }
    
    // Render menu bar
    RenderMainMenuBar();
    
    ImGui::End();
    
    // Render individual panels as separate windows
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
    
    if (panelVisibility.viewport)
        RenderViewport();
    
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
    
    // Update and render additional platform windows (for multi-viewport support)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

// Setup default docking layout
void EditorUI::SetupDefaultDockingLayout(ImGuiID dockspaceId)
{
    // Clear any existing layout
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);
    
    // Split the dockspace into regions
    ImGuiID dock_main_id = dockspaceId;
    ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
    ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
    ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
    
    // Dock windows to their default positions
    ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
    ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
    ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Scene Stats", dock_id_bottom);
    ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
    ImGui::DockBuilderDockWindow("Toolbar", dock_id_left);
    
    ImGui::DockBuilderFinish(dockspaceId);
}

// Render main menu bar
void EditorUI::RenderMainMenuBar()
{
    if (ImGui::BeginMenuBar())
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
        
        if (ImGui::BeginMenu("Actor"))
        {
            if (ImGui::MenuItem("Create Empty Actor"))
            {
                showCreateActorPopup = true;
            }
            ImGui::Separator();
            
            if (ImGui::BeginMenu("3D Object"))
            {
                if (ImGui::MenuItem("Cube"))
                {
                    Actor* cubeActor = new Actor("Cube");
                    // Add renderer component when available
                    SceneManager::GetInstance().AddActor(cubeActor);
                    AddConsoleLog("Created Cube actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                if (ImGui::MenuItem("Sphere"))
                {
                    Actor* sphereActor = new Actor("Sphere");
                    // Add renderer component when available
                    SceneManager::GetInstance().AddActor(sphereActor);
                    AddConsoleLog("Created Sphere actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                if (ImGui::MenuItem("Plane"))
                {
                    Actor* planeActor = new Actor("Plane");
                    // Add renderer component when available
                    SceneManager::GetInstance().AddActor(planeActor);
                    AddConsoleLog("Created Plane actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Light"))
            {
                if (ImGui::MenuItem("Directional Light"))
                {
                    Actor* lightActor = new Actor("Directional Light");
                    SceneManager::GetInstance().AddActor(lightActor);
                    AddConsoleLog("Created Directional Light actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                if (ImGui::MenuItem("Point Light"))
                {
                    Actor* lightActor = new Actor("Point Light");
                    SceneManager::GetInstance().AddActor(lightActor);
                    AddConsoleLog("Created Point Light actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                if (ImGui::MenuItem("Spot Light"))
                {
                    Actor* lightActor = new Actor("Spot Light");
                    SceneManager::GetInstance().AddActor(lightActor);
                    AddConsoleLog("Created Spot Light actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::MenuItem("Camera"))
            {
                Actor* cameraActor = new Actor("Camera");
                SceneManager::GetInstance().AddActor(cameraActor);
                AddConsoleLog("Created Camera actor", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Toolbar", nullptr, &panelVisibility.toolbar);
            ImGui::MenuItem("Hierarchy", nullptr, &panelVisibility.hierarchy);
            ImGui::MenuItem("Inspector", nullptr, &panelVisibility.inspector);
            ImGui::MenuItem("Viewport", nullptr, &panelVisibility.viewport);
            ImGui::MenuItem("Scene Stats", nullptr, &panelVisibility.sceneStats);
            ImGui::MenuItem("Console", nullptr, &panelVisibility.console);
            ImGui::Separator();
            if (ImGui::MenuItem("Reset Layout"))
            {
                isDockingInitialized = false;
                AddConsoleLog("Docking layout will reset on next frame", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
            }
            ImGui::Separator();
            ImGui::MenuItem("ImGui Demo", "F1", &showDemoWindow);
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {
                AddConsoleLog("Ice Crystal Engine v1.0", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
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
    ImGui::SetNextWindowSize(ImVec2(800, 80), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Toolbar", &panelVisibility.toolbar))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));

        ImGui::SameLine(20);
        
        if (playMode == PlayMode::EDIT)
        {
            // In EDIT mode - show Play button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button("Play", ImVec2(80, 30)))
            {
                playMode = PlayMode::PLAY;
                AddConsoleLog("Entering Play Mode", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
            
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Start game simulation (Ctrl+P)");
        }
        else if (playMode == PlayMode::PLAY)
        {
            // In PLAY mode - show Pause button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
            if (ImGui::Button("Pause", ImVec2(80, 30)))
            {
                playMode = PlayMode::PAUSED;
                AddConsoleLog("Game Paused", glm::vec4(1.0f, 1.0f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
        }
        else // PAUSED
        {
            // In PAUSED mode - show Resume button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button("▶ Resume", ImVec2(80, 30)))
            {
                playMode = PlayMode::PLAY;
                AddConsoleLog("Game Resumed", glm::vec4(0.4f, 1.0f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
        }
        
        // Stop button (only visible when playing or paused)
        if (playMode != PlayMode::EDIT)
        {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button("Stop", ImVec2(80, 30)))
            {
                playMode = PlayMode::EDIT;
                AddConsoleLog("Stopped - Returning to Edit Mode", glm::vec4(1.0f, 0.6f, 0.4f, 1.0f));
            }
            ImGui::PopStyleColor();
            
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Stop game and return to editor");
        }
        
        // Separator
        ImGui::SameLine();
        ImGui::Text("|");
        
        // Gizmo controls (only in Edit mode)
        if (playMode == PlayMode::EDIT)
        {
            ImGui::SameLine();
            ImGui::Text("Gizmo:");
        
            ImGui::SameLine();
            GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
        
            ImGui::SameLine();
            bool wasEnabled = gizmoRenderer.IsEnabled();
            if (wasEnabled)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
            if (ImGui::Button(wasEnabled ? "Hide (G)" : "Show (G)", ImVec2(85, 30)))
            {
                gizmoRenderer.SetEnabled(!wasEnabled);
            }
            if (wasEnabled)
                ImGui::PopStyleColor();
        }
        
        ImGui::PopStyleVar(2);
    }
    ImGui::End();
}

// Render hierarchy panel
void EditorUI::RenderHierarchy()
{
    SceneManager& sceneManager = SceneManager::GetInstance();
    
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Hierarchy", &panelVisibility.hierarchy))
    {
        
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
    ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Inspector", &panelVisibility.inspector))
    {
        
        if (selectedActor)
        {
            // Actor name
            ImGui::PushFont(boldFont ? boldFont : defaultFont);
            ImGui::Text("%s", selectedActor->name.c_str());
            ImGui::PopFont();
            
            // Delete and Duplicate buttons
            ImGui::SameLine(ImGui::GetWindowWidth() - 180);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
            if (ImGui::Button("Delete", ImVec2(80, 0)))
            {
                SceneManager::GetInstance().RemoveActor(selectedActor);
                AddConsoleLog("Deleted actor: " + selectedActor->name, glm::vec4(1.0f, 0.6f, 0.4f, 1.0f));
                selectedActor = nullptr;
            }
            ImGui::PopStyleColor();
            
            ImGui::SameLine();
            if (ImGui::Button("Duplicate", ImVec2(80, 0)))
            {
                AddConsoleLog("Duplicate not implemented yet", glm::vec4(1.0f, 1.0f, 0.4f, 1.0f));
            }
            
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
        
        // Reset buttons
        ImGui::Spacing();
        if (ImGui::Button("Reset Position", ImVec2(150, 0)))
        {
            transform->SetPosition(glm::vec3(0.0f));
            AddConsoleLog("Reset position", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Rotation", ImVec2(150, 0)))
        {
            transform->SetRotation(glm::vec3(0.0f));
            AddConsoleLog("Reset rotation", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Scale", ImVec2(150, 0)))
        {
            transform->SetScale(glm::vec3(1.0f));
            AddConsoleLog("Reset scale", glm::vec4(0.4f, 0.8f, 1.0f, 1.0f));
        }
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
// Render scene statistics
void EditorUI::RenderSceneStats()
{
    SceneManager& sceneManager = SceneManager::GetInstance();
    LightingManager& lightingManager = LightingManager::GetInstance();
    
    ImGui::SetNextWindowSize(ImVec2(400, 250), ImGuiCond_FirstUseEver);
    
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
        {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.47f, 1.0f), "Hovered:");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s", hoveredActor->name.c_str());
            
            // Show if hovered actor can be selected
            if (isMouseInViewport && hoveredActor != selectedActor)
            {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Click to select)");
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Hovered: None");
        }
        
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
// Render console
void EditorUI::RenderConsole()
{
    ImGui::SetNextWindowSize(ImVec2(600, 200), ImGuiCond_FirstUseEver);
    
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

// Render viewport (scene view)
void EditorUI::RenderViewport()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Viewport", &panelVisibility.viewport))
    {
        // Get the size of the viewport window
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Get the position of the viewport window (top-left corner in screen space)
        ImVec2 viewportMin = ImGui::GetWindowContentRegionMin();
        ImVec2 viewportMax = ImGui::GetWindowContentRegionMax();
        viewportMin.x += ImGui::GetWindowPos().x;
        viewportMin.y += ImGui::GetWindowPos().y;
        viewportMax.x += ImGui::GetWindowPos().x;
        viewportMax.y += ImGui::GetWindowPos().y;
        
        // Store viewport position
        viewportPosition = glm::vec2(viewportMin.x, viewportMin.y);
        
        // Check if mouse is hovering over the viewport
        ImVec2 mousePos = ImGui::GetMousePos();
        isMouseInViewport = (mousePos.x >= viewportMin.x && mousePos.x <= viewportMax.x &&
                            mousePos.y >= viewportMin.y && mousePos.y <= viewportMax.y);
        
        // Calculate mouse position relative to viewport (0,0 = top-left of viewport)
        if (isMouseInViewport)
        {
            viewportMousePos = glm::vec2(mousePos.x - viewportMin.x, mousePos.y - viewportMin.y);
        }
        
        // Check if we need to resize the framebuffer
        if (viewportSize.x > 0 && viewportSize.y > 0)
        {
            int newWidth = (int)viewportSize.x;
            int newHeight = (int)viewportSize.y;
            
            if (viewportWidth != newWidth || viewportHeight != newHeight)
            {
                ResizeViewportFramebuffer(newWidth, newHeight);
            }
            
            // Display the framebuffer texture
            if (viewportTexture != 0)
            {
                ImGui::Image((ImTextureID)(intptr_t)viewportTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
                
                // Draw overlay information on top of viewport
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                
                // Draw focus border if viewport is focused (for editor camera)
                #ifdef _DEBUG
                if (playMode == PlayMode::EDIT)
                {
                    EditorCamera& editorCam = EditorCamera::GetInstance();
                    if (editorCam.IsViewportFocused())
                    {
                        // Draw a bright border to indicate viewport has camera focus
                        drawList->AddRect(
                            ImVec2(viewportMin.x, viewportMin.y),
                            ImVec2(viewportMax.x, viewportMax.y),
                            IM_COL32(100, 200, 255, 255),
                            0.0f,
                            0,
                            3.0f
                        );
                    }
                    else if (isMouseInViewport)
                    {
                        // Draw a subtle border when hovering (ready to focus)
                        drawList->AddRect(
                            ImVec2(viewportMin.x, viewportMin.y),
                            ImVec2(viewportMax.x, viewportMax.y),
                            IM_COL32(100, 200, 255, 80),
                            0.0f,
                            0,
                            2.0f
                        );
                    }
                }
                #endif
                
                // Show current mode indicator
                ImVec2 textPos(viewportMin.x + 10, viewportMin.y + 10);
                
                std::string modeText;
                ImU32 modeColor;
                if (playMode == PlayMode::EDIT)
                {
                    modeText = "EDIT MODE";
                    modeColor = IM_COL32(100, 200, 255, 255);
                }
                else if (playMode == PlayMode::PLAY)
                {
                    modeText = "PLAYING";
                    modeColor = IM_COL32(100, 255, 100, 255);
                }
                else // PAUSED
                {
                    modeText = "PAUSED";
                    modeColor = IM_COL32(255, 200, 100, 255);
                }
                
                drawList->AddRectFilled(
                    ImVec2(textPos.x - 5, textPos.y - 2),
                    ImVec2(textPos.x + 120, textPos.y + 20),
                    IM_COL32(0, 0, 0, 180)
                );
                drawList->AddText(textPos, modeColor, modeText.c_str());
                textPos.y += 22;
                
                // Show camera focus status in Edit mode
                #ifdef _DEBUG
                if (playMode == PlayMode::EDIT)
                {
                    EditorCamera& editorCam = EditorCamera::GetInstance();
                    if (editorCam.IsViewportFocused())
                    {
                        drawList->AddRectFilled(
                            ImVec2(textPos.x - 5, textPos.y - 2),
                            ImVec2(textPos.x + 130, textPos.y + 20),
                            IM_COL32(0, 0, 0, 180)
                        );
                        drawList->AddText(textPos, IM_COL32(100, 200, 255, 255), "Camera Focused");
                        textPos.y += 22;
                    }
                }
                #endif
                
                // Show gizmo controls if in EDIT mode and gizmos are enabled
                GizmoRenderer& gizmoRenderer = GizmoRenderer::GetInstance();
                if (playMode == PlayMode::EDIT && gizmoRenderer.IsEnabled())
                {
                    // Gizmo mode indicator
                    std::string modeText = "Gizmo: ";
                    GizmoMode mode = gizmoRenderer.GetMode();
                    if (mode == GizmoMode::Translate)
                        modeText += "Translate (W)";
                    else if (mode == GizmoMode::Rotate)
                        modeText += "Rotate (E)";
                    else if (mode == GizmoMode::Scale)
                        modeText += "Scale (R)";
                    
                    drawList->AddRectFilled(
                        ImVec2(textPos.x - 5, textPos.y - 2),
                        ImVec2(textPos.x + 150, textPos.y + 20),
                        IM_COL32(0, 0, 0, 180)
                    );
                    drawList->AddText(textPos, IM_COL32(100, 200, 255, 255), modeText.c_str());
                    
                    // Show if dragging
                    if (gizmoRenderer.IsDragging())
                    {
                        textPos.y += 25;
                        drawList->AddRectFilled(
                            ImVec2(textPos.x - 5, textPos.y - 2),
                            ImVec2(textPos.x + 90, textPos.y + 20),
                            IM_COL32(0, 0, 0, 180)
                        );
                        drawList->AddText(textPos, IM_COL32(255, 255, 100, 255), "Dragging...");
                    }
                }
                
                // Show hovered actor info (above selected actor info)
                SceneManager& sceneManager = SceneManager::GetInstance();
                Actor* hoveredActor = sceneManager.GetHoveredActor();
                
                if (hoveredActor != nullptr && isMouseInViewport)
                {
                    // Position above mouse cursor
                    ImVec2 hoverTextPos(viewportMin.x + viewportMousePos.x + 15, viewportMin.y + viewportMousePos.y - 25);
                    
                    // Ensure it stays within viewport bounds
                    if (hoverTextPos.x + 200 > viewportMax.x)
                        hoverTextPos.x = viewportMax.x - 200;
                    if (hoverTextPos.y < viewportMin.y)
                        hoverTextPos.y = viewportMin.y + 5;
                    
                    std::string hoverText = hoveredActor->name;
                    float textWidth = hoverText.length() * 7 + 10;
                    
                    // Draw background with slight transparency
                    drawList->AddRectFilled(
                        ImVec2(hoverTextPos.x - 5, hoverTextPos.y - 2),
                        ImVec2(hoverTextPos.x + textWidth, hoverTextPos.y + 20),
                        IM_COL32(40, 40, 45, 220),
                        4.0f
                    );
                    
                    // Draw border
                    drawList->AddRect(
                        ImVec2(hoverTextPos.x - 5, hoverTextPos.y - 2),
                        ImVec2(hoverTextPos.x + textWidth, hoverTextPos.y + 20),
                        IM_COL32(255, 200, 100, 255),
                        4.0f,
                        0,
                        2.0f
                    );
                    
                    // Draw text
                    drawList->AddText(hoverTextPos, IM_COL32(255, 220, 120, 255), hoverText.c_str());
                    
                    // If hovered actor is different from selected, show "Click to select"
                    if (hoveredActor != selectedActor)
                    {
                        ImVec2 clickHintPos(hoverTextPos.x, hoverTextPos.y + 22);
                        const char* clickHint = "Click to select";
                        float hintWidth = strlen(clickHint) * 6 + 10;
                        
                        drawList->AddRectFilled(
                            ImVec2(clickHintPos.x - 5, clickHintPos.y - 2),
                            ImVec2(clickHintPos.x + hintWidth, clickHintPos.y + 18),
                            IM_COL32(30, 30, 35, 200),
                            4.0f
                        );
                        drawList->AddText(clickHintPos, IM_COL32(180, 180, 180, 255), clickHint);
                    }
                }
                
                // Show selected actor info
                if (selectedActor != nullptr)
                {
                    ImVec2 textPos(viewportMin.x + 10, viewportMax.y - 50);
                    std::string actorInfo = "Selected: " + selectedActor->name;
                    
                    drawList->AddRectFilled(
                        ImVec2(textPos.x - 5, textPos.y - 2),
                        ImVec2(textPos.x + actorInfo.length() * 7 + 10, textPos.y + 20),
                        IM_COL32(0, 0, 0, 180)
                    );
                    drawList->AddText(textPos, IM_COL32(100, 255, 100, 255), actorInfo.c_str());
                    
                    // Show transform info
                    textPos.y += 25;
                    glm::vec3 pos = selectedActor->transform->position;
                    char posText[128];
                    snprintf(posText, sizeof(posText), "Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
                    
                    drawList->AddRectFilled(
                        ImVec2(textPos.x - 5, textPos.y - 2),
                        ImVec2(textPos.x + 250, textPos.y + 20),
                        IM_COL32(0, 0, 0, 180)
                    );
                    drawList->AddText(textPos, IM_COL32(200, 200, 200, 255), posText);
                }
            }
            else
            {
                // Fallback if texture isn't ready
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Scene Viewport");
                ImGui::Text("Viewport Size: %.0f x %.0f", viewportSize.x, viewportSize.y);
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Framebuffer texture not initialized!");
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Scene Viewport");
            ImGui::Text("Resize window to initialize viewport");
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

// Create viewport framebuffer
void EditorUI::CreateViewportFramebuffer(int width, int height)
{
    viewportWidth = width;
    viewportHeight = height;
    
    // Generate framebuffer
    glGenFramebuffers(1, &viewportFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, viewportFBO);
    
    // Generate texture for color attachment 0 (scene rendering)
    glGenTextures(1, &viewportTexture);
    glBindTexture(GL_TEXTURE_2D, viewportTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, viewportTexture, 0);
    
    // Generate texture for color attachment 1 (bright color for bloom)
    glGenTextures(1, &viewportBrightTexture);
    glBindTexture(GL_TEXTURE_2D, viewportBrightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, viewportBrightTexture, 0);
    
    // Generate texture for color attachment 2 (actor picking - unique colors)
    glGenTextures(1, &viewportPickingTexture);
    glBindTexture(GL_TEXTURE_2D, viewportPickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, viewportPickingTexture, 0);
    
    // Set draw buffers to render to all three attachments
    GLenum drawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, drawBuffers);
    
    // Generate renderbuffer for depth and stencil attachment
    glGenRenderbuffers(1, &viewportRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, viewportRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, viewportRBO);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR: Viewport framebuffer is not complete!" << std::endl;
        AddConsoleLog("ERROR: Viewport framebuffer creation failed!", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    }
    else
    {
        std::cout << "Viewport framebuffer created successfully: " << width << "x" << height << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Resize viewport framebuffer
void EditorUI::ResizeViewportFramebuffer(int width, int height)
{
    if (width <= 0 || height <= 0) return;
    
    // Delete old framebuffer if it exists
    if (viewportFBO != 0)
    {
        DeleteViewportFramebuffer();
    }
    
    // Create new framebuffer with new size
    CreateViewportFramebuffer(width, height);
}

// Delete viewport framebuffer
void EditorUI::DeleteViewportFramebuffer()
{
    if (viewportFBO != 0)
    {
        glDeleteFramebuffers(1, &viewportFBO);
        viewportFBO = 0;
    }
    
    if (viewportTexture != 0)
    {
        glDeleteTextures(1, &viewportTexture);
        viewportTexture = 0;
    }
    
    if (viewportBrightTexture != 0)
    {
        glDeleteTextures(1, &viewportBrightTexture);
        viewportBrightTexture = 0;
    }
    
    if (viewportPickingTexture != 0)
    {
        glDeleteTextures(1, &viewportPickingTexture);
        viewportPickingTexture = 0;
    }
    
    if (viewportRBO != 0)
    {
        glDeleteRenderbuffers(1, &viewportRBO);
        viewportRBO = 0;
    }
    
    viewportWidth = 0;
    viewportHeight = 0;
}

// Get the picked actor color from viewport picking texture
glm::vec3 EditorUI::GetViewportPickedColor() const
{
    if (viewportFBO == 0 || !isMouseInViewport)
    {
        return glm::vec3(0.0f); // No valid pick
    }
    
    // Bind the viewport framebuffer for reading
    glBindFramebuffer(GL_READ_FRAMEBUFFER, viewportFBO);
    glReadBuffer(GL_COLOR_ATTACHMENT2); // Picking texture
    
    // Read pixel at mouse position (viewport-relative coordinates)
    float buffer[3];
    int x = (int)viewportMousePos.x;
    // ImGui uses top-left origin (0,0 at top), OpenGL uses bottom-left origin (0,0 at bottom)
    // ImGui::Image displays with UV (0,1) to (1,0), which flips the texture vertically
    // So viewportMousePos is in ImGui space (top=0), but glReadPixels expects OpenGL space (bottom=0)
    // Since the image is displayed flipped, we need to flip the Y coordinate back
    int y = viewportHeight - 1 - (int)viewportMousePos.y;
    
    // Clamp to viewport bounds
    x = glm::clamp(x, 0, viewportWidth - 1);
    y = glm::clamp(y, 0, viewportHeight - 1);
    
    glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, buffer);
    
    // Unbind framebuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    
    // Decode color
    glm::vec3 decodedColor = glm::vec3(buffer[0], buffer[1], buffer[2]);
    decodedColor *= 255.0f;
    decodedColor.x = round(decodedColor.x);
    decodedColor.y = round(decodedColor.y);
    decodedColor.z = round(decodedColor.z);
    
    return decodedColor;
}

#endif // ICE_EDITOR
