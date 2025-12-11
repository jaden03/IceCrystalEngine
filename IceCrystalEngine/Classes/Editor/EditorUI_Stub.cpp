#include <Ice/Editor/EditorUI.h>
#include <Ice/Core/Actor.h>

// Stub implementation for EditorUI
// This provides minimal functionality when editor is not linked
// The actual implementation is in IceCrystalEngineEditor project

#ifndef ICE_EDITOR

namespace
{
    class EditorUIStub : public EditorUI
    {
    public:
        EditorUIStub()
            : playMode(PlayMode::PLAY)  // Always in PLAY mode for Release builds
            , selectedActor(nullptr)
            , viewportFBO(0)
            , viewportWidth(1280)
            , viewportHeight(720)
            , isMouseInViewport(false)
            , viewportMousePos(0.0f, 0.0f)
        {}
        ~EditorUIStub() = default;
        
    private:
        PlayMode playMode;
        Actor* selectedActor;
        unsigned int viewportFBO;
        int viewportWidth;
        int viewportHeight;
        bool isMouseInViewport;
        glm::vec2 viewportMousePos;
    };
}

EditorUI& EditorUI::GetInstance()
{
    static EditorUIStub instance;
    return instance;
}

// Stub method implementations (no-op)
void EditorUI::Initialize() {}
void EditorUI::Cleanup() {}

void EditorUI::BeginFrame() {}
void EditorUI::RenderEditor() {}
void EditorUI::EndFrame() {}

void EditorUI::SetPlayMode(PlayMode mode) {}
PlayMode EditorUI::GetPlayMode() const { return PlayMode::PLAY; }  // Always PLAY in Release
bool EditorUI::IsEditMode() const { return false; }  // Never in edit mode in Release
bool EditorUI::IsPlayMode() const { return true; }   // Always playing in Release
bool EditorUI::IsGamePaused() const { return false; }

void EditorUI::SetEnginePaused(bool paused) {}
bool EditorUI::IsEnginePaused() const { return false; }

void EditorUI::SetSelectedActor(Actor* actor) {}
Actor* EditorUI::GetSelectedActor() const { return nullptr; }
bool EditorUI::HasSelectedActor() const { return false; }

void EditorUI::AddConsoleLog(const std::string& message, const glm::vec4& color) {}

unsigned int EditorUI::GetViewportFramebuffer() const { return 0; }
int EditorUI::GetViewportWidth() const { return 1280; }
int EditorUI::GetViewportHeight() const { return 720; }
bool EditorUI::IsViewportActive() const { return false; }

bool EditorUI::IsMouseInViewport() const { return false; }
glm::vec2 EditorUI::GetViewportMousePos() const { return glm::vec2(0.0f, 0.0f); }
glm::vec2 EditorUI::GetViewportSize() const { return glm::vec2(1280.0f, 720.0f); }

glm::vec3 EditorUI::GetViewportPickedColor() const { return glm::vec3(0.0f); }

#endif // ICE_EDITOR