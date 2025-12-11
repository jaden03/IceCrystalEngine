#include <Ice/Editor/EditorCamera.h>

// Stub implementation for EditorCamera
// This provides minimal functionality when editor is not linked
// The actual implementation is in IceCrystalEngineEditor project

#ifndef ICE_EDITOR

namespace
{
    class EditorCameraStub : public EditorCamera
    {
    public:
        EditorCameraStub()
        {
            position = glm::vec3(0.0f, 5.0f, 10.0f);
            forward = glm::vec3(0.0f, 0.0f, -1.0f);
            up = glm::vec3(0.0f, 1.0f, 0.0f);
            right = glm::vec3(1.0f, 0.0f, 0.0f);
            
            view = glm::mat4(1.0f);
            projection = glm::mat4(1.0f);
            
            fov = 45.0f;
            nearPlane = 0.1f;
            farPlane = 1000.0f;
            aspectRatio = 16.0f / 9.0f;
            
            yaw = -90.0f;
            pitch = 0.0f;
            
            moveSpeed = 5.0f;
            lookSpeed = 0.15f;
            boostMultiplier = 3.0f;
        }
        ~EditorCameraStub() = default;
    };
}

EditorCamera& EditorCamera::GetInstance()
{
    static EditorCameraStub instance;
    return instance;
}

// Stub method implementations (no-op)
void EditorCamera::Initialize(const glm::vec3& initialPosition) {}
void EditorCamera::Update(float deltaTime, bool isMouseInViewport, const glm::vec2& mousePos) {}
void EditorCamera::UpdateViewMatrix() {}
void EditorCamera::UpdateProjectionMatrix(float width, float height) {}
void EditorCamera::UpdateVectors() {}

void EditorCamera::SetViewportFocused(bool focused) {}
bool EditorCamera::IsViewportFocused() const { return false; }

void EditorCamera::ProcessMouseMovement(float deltaX, float deltaY) {}
void EditorCamera::ProcessKeyboardMovement(const glm::vec3& direction, float deltaTime, bool boost) {}
void EditorCamera::FocusOn(const glm::vec3& focusPoint) {}
void EditorCamera::Reset() {}

glm::vec3 EditorCamera::GetForward() const { return forward; }
glm::vec3 EditorCamera::GetRight() const { return right; }
glm::vec3 EditorCamera::GetUp() const { return up; }

#endif // ICE_EDITOR