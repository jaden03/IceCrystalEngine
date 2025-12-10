#pragma once

#ifndef EDITOR_UI_H
#define EDITOR_UI_H

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class Actor;
class Component;
class Transform;
enum class GizmoMode;
enum class GizmoAxis;

class EditorUI
{
public:
    static EditorUI& GetInstance()
    {
        static EditorUI instance;
        return instance;
    }

    // Initialize and cleanup
    void Initialize();
    void Cleanup();

    // Frame lifecycle
    void BeginFrame();
    void RenderEditor();
    void EndFrame();

    // Editor state
    void SetEnabled(bool enabled) { editorEnabled = enabled; }
    bool IsEnabled() const { return editorEnabled; }
    
    void SetEnginePaused(bool paused) { enginePaused = paused; }
    bool IsEnginePaused() const { return enginePaused; }

    // Actor selection
    void SetSelectedActor(Actor* actor) { selectedActor = actor; }
    Actor* GetSelectedActor() const { return selectedActor; }
    bool HasSelectedActor() const { return selectedActor != nullptr; }

    // Panel visibility
    struct PanelVisibility
    {
        bool hierarchy = true;
        bool inspector = true;
        bool sceneStats = true;
        bool toolbar = true;
        bool console = true;
        bool viewport = false; // Not implemented yet
    };
    
    PanelVisibility panelVisibility;

    // Console functionality (public for Lua and other systems to log)
    void AddConsoleLog(const std::string& message, const glm::vec4& color = glm::vec4(1.0f));
    void ClearConsole();

private:
    EditorUI();
    ~EditorUI();
    EditorUI(EditorUI const&) = delete;
    void operator=(EditorUI const&) = delete;

    // Panel rendering methods
    void RenderMainMenuBar();
    void RenderToolbar();
    void RenderHierarchy();
    void RenderInspector();
    void RenderSceneStats();
    void RenderConsole();
    
    // Inspector sub-panels
    void RenderTransformInspector(Transform* transform);
    void RenderComponentInspector(Component* component);
    void RenderAddComponentMenu();

    // Hierarchy helpers
    void RenderActorNode(Actor* actor, int index);
    void HandleActorSelection(Actor* actor);
    
    // Utility methods
    std::string GetComponentTypeName(Component* component);
    void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
    void DrawFloatControl(const std::string& label, float& value, float resetValue = 0.0f, float speed = 0.1f);
    void DrawColorControl(const std::string& label, glm::vec3& color);

    // Member variables
    bool editorEnabled;
    bool enginePaused;
    Actor* selectedActor;
    
    // Fonts
    ImFont* defaultFont;
    ImFont* boldFont;
    ImFont* titleFont;

    // UI State
    bool showDemoWindow;
    float hierarchyWidth;
    float inspectorWidth;
    float toolbarHeight;
    
    // Console
    struct ConsoleMessage
    {
        std::string message;
        glm::vec4 color;
        float timestamp;
    };
    std::vector<ConsoleMessage> consoleMessages;
    char consoleInputBuffer[256];
    bool consoleAutoScroll;
    
    // Gizmo UI state
    int currentGizmoMode; // 0 = Translate, 1 = Rotate, 2 = Scale
    bool gizmosEnabled;
    
    // Actor creation
    bool showCreateActorPopup;
    char newActorNameBuffer[128];
    
    // Style
    void SetupEditorStyle();
    ImVec4 accentColor;
    ImVec4 highlightColor;
    ImVec4 backgroundColor;
    ImVec4 windowBgColor;
    
    // Performance tracking
    float deltaTime;
    float fps;
    std::vector<float> fpsHistory;
    const size_t fpsHistorySize = 90;
};

#endif