#pragma once

#include <glm/glm.hpp>
#include <string>

// Forward declarations
class Actor;

// Play mode states for the editor
enum class PlayMode
{
    EDIT,    // Editor mode - game not running, can edit scene
    PLAY,    // Play mode - game running, testing
    PAUSED   // Play mode paused - can inspect state but game is frozen
};

// Stub header for EditorUI - actual implementation is in IceCrystalEngineEditor project
// This allows the engine to compile without editor dependencies
class EditorUI
{
public:
    static EditorUI& GetInstance();
    
    // Initialization and cleanup
    void Initialize();
    void Cleanup();
    
    // Frame lifecycle
    void BeginFrame();
    void RenderEditor();
    void EndFrame();
    
    // Play mode control
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
    
    // Console functionality
    void AddConsoleLog(const std::string& message, const glm::vec4& color = glm::vec4(1.0f));
    
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
};