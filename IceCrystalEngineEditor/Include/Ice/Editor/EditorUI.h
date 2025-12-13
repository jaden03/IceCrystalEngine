#pragma once

#ifndef EDITOR_UI_H
#define EDITOR_UI_H

#include <glad/glad.h>
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
class EditorCamera;
enum class GizmoMode;
enum class GizmoAxis;

// Play mode states for the editor
enum class PlayMode
{
    EDIT,    // Editor mode - game not running, can edit scene
    PLAY,    // Play mode - game running, testing
    PAUSED   // Play mode paused - can inspect state but game is frozen
};

class EditorUI
{
public:
    static EditorUI& GetInstance();

    // Initialize and cleanup
    void Initialize();
    void Cleanup();

    // Frame lifecycle
    void BeginFrame();
    void RenderEditor();
    void EndFrame();

    // Editor state
    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    
    // Play mode control (replaces pause/unpause)
    void SetPlayMode(PlayMode mode);
    PlayMode GetPlayMode() const;
    bool IsEditMode() const;
    bool IsPlayMode() const;
    bool IsGamePaused() const;
    
    // Legacy compatibility
    void SetEnginePaused(bool paused);
    bool IsEnginePaused() const;

    // Actor selection
    void SetSelectedActor(Actor* actor);
    Actor* GetSelectedActor() const;
    bool HasSelectedActor() const;

    // Panel visibility
    struct PanelVisibility
    {
        bool hierarchy = true;
        bool inspector = true;
        bool sceneStats = true;
        bool toolbar = true;
        bool console = true;
        bool viewport = true;
    };
    
    PanelVisibility panelVisibility;

    // Console functionality (public for Lua and other systems to log)
    void AddConsoleLog(const std::string& message, const glm::vec4& color = glm::vec4(1.0f));
    void ClearConsole();
    
    // Viewport framebuffer access
    unsigned int GetViewportFramebuffer() const;
    int GetViewportWidth() const;
    int GetViewportHeight() const;
    bool IsViewportActive() const;
    
    // Viewport mouse interaction
    bool IsMouseInViewport() const;
    glm::vec2 GetViewportMousePos() const;
    glm::vec2 GetViewportSize() const;
    
    // Viewport actor picking
    glm::vec3 GetViewportPickedColor() const;
    
    // Actor selection in viewport
    void HandleViewportClick();

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
    void RenderViewport();
    
    // Docking layout
    void SetupDefaultDockingLayout(ImGuiID dockspaceId);
    
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
    PlayMode playMode;  // Current play mode state
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
    
    // Docking
    bool isDockingInitialized;
    
    // Viewport framebuffer
    unsigned int viewportFBO;
    unsigned int viewportTexture;
    unsigned int viewportBrightTexture;   // For bloom/bright colors
    unsigned int viewportPickingTexture;  // For actor picking (unique colors)
    unsigned int viewportRBO;
    int viewportWidth;
    int viewportHeight;
    
    // Viewport mouse tracking
    bool isMouseInViewport;
    glm::vec2 viewportMousePos;
    glm::vec2 viewportPosition;
    
    void CreateViewportFramebuffer(int width, int height);
    void ResizeViewportFramebuffer(int width, int height);
    void DeleteViewportFramebuffer();
};

#endif