#include <Ice/IEditor/GizmoRenderer.h>

// Stub implementation for GizmoRenderer
// This provides minimal functionality when editor is not linked
// The actual implementation is in IceCrystalEngineEditor project

#ifndef ICE_EDITOR

namespace
{
    class GizmoRendererStub : public GizmoRenderer
    {
    public:
        GizmoRendererStub() = default;
        ~GizmoRendererStub() = default;
    };
}

GizmoRenderer& GizmoRenderer::GetInstance()
{
    static GizmoRendererStub instance;
    return instance;
}

// Stub method implementations (no-op)
void GizmoRenderer::Initialize() {}

void GizmoRenderer::SetMode(GizmoMode mode) {}
GizmoMode GizmoRenderer::GetMode() const { return GizmoMode::Translate; }

void GizmoRenderer::SetEnabled(bool enabled) {}
bool GizmoRenderer::IsEnabled() const { return false; }

bool GizmoRenderer::IsDragging() const { return false; }

GizmoAxis GizmoRenderer::GetHoveredAxis(const glm::vec2& mousePos, const glm::vec2& screenSize,
                        const glm::mat4& view, const glm::mat4& projection,
                        const glm::vec3& gizmoPosition) { return GizmoAxis::None; }

void GizmoRenderer::HandleMouseDown(const glm::vec2& mousePos, const glm::vec2& screenSize,
                    const glm::mat4& view, const glm::mat4& projection,
                    Actor* actor) {}

void GizmoRenderer::HandleMouseMove(const glm::vec2& mousePos, const glm::vec2& screenSize,
                    const glm::mat4& view, const glm::mat4& projection) {}

void GizmoRenderer::HandleMouseUp() {}

void GizmoRenderer::RenderGizmos(Actor* actor, Camera* camera) {}

#endif // ICE_EDITOR