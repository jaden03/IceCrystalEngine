#include <Ice/Editor/GizmoRenderer.h>
#include <Ice/Core/Actor.h>
#include <iostream>
#include <Ice/Components/Camera.h>
#include <Ice/Core/WindowManager.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

GizmoRenderer::GizmoRenderer()
    : lineShaderProgram(0)
    , arrowVAO(0), arrowVBO(0)
    , circleVAO(0), circleVBO(0)
    , cubeVAO(0), cubeVBO(0)
    , currentMode(GizmoMode::Translate)
    , enabled(false)
    , isDragging(false)
    , dragAxis(GizmoAxis::None)
    , hoveredAxis(GizmoAxis::None)
    , dragActor(nullptr)
{
}

GizmoRenderer::~GizmoRenderer()
{
    Cleanup();
}

void GizmoRenderer::Initialize()
{
    std::cout << "[GizmoRenderer] Initializing..." << std::endl;
    InitializeShaders();
    InitializeGeometry();
    enabled = true;
    std::cout << "[GizmoRenderer] Initialization complete. Enabled: " << enabled << std::endl;
}

void GizmoRenderer::Cleanup()
{
    if (lineShaderProgram) glDeleteProgram(lineShaderProgram);
    if (arrowVAO) glDeleteVertexArrays(1, &arrowVAO);
    if (arrowVBO) glDeleteBuffers(1, &arrowVBO);
    if (circleVAO) glDeleteVertexArrays(1, &circleVAO);
    if (circleVBO) glDeleteBuffers(1, &circleVBO);
    if (cubeVAO) glDeleteVertexArrays(1, &cubeVAO);
    if (cubeVBO) glDeleteBuffers(1, &cubeVBO);
}

void GizmoRenderer::InitializeShaders()
{
    std::cout << "[GizmoRenderer] Initializing shaders..." << std::endl;
    // Simple line shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat4 model;
        
        void main()
        {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec3 color;
        
        void main()
        {
            FragColor = vec4(color, 1.0);
        }
    )";

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "[GizmoRenderer] Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "[GizmoRenderer] Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Link shader program
    lineShaderProgram = glCreateProgram();
    glAttachShader(lineShaderProgram, vertexShader);
    glAttachShader(lineShaderProgram, fragmentShader);
    glLinkProgram(lineShaderProgram);

    glGetProgramiv(lineShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(lineShaderProgram, 512, NULL, infoLog);
        std::cerr << "[GizmoRenderer] Shader program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    std::cout << "[GizmoRenderer] Shaders initialized. Program ID: " << lineShaderProgram << std::endl;
}

void GizmoRenderer::InitializeGeometry()
{
    // Arrow geometry (line + cone for head)
    float arrowVertices[] = {
        // Line shaft
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        
        // Arrow head (simple cone approximation with lines)
        0.0f, 1.0f, 0.0f,
        0.05f, 0.85f, 0.0f,
        
        0.0f, 1.0f, 0.0f,
        -0.05f, 0.85f, 0.0f,
        
        0.0f, 1.0f, 0.0f,
        0.0f, 0.85f, 0.05f,
        
        0.0f, 1.0f, 0.0f,
        0.0f, 0.85f, -0.05f,
    };

    glGenVertexArrays(1, &arrowVAO);
    glGenBuffers(1, &arrowVBO);
    
    glBindVertexArray(arrowVAO);
    glBindBuffer(GL_ARRAY_BUFFER, arrowVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrowVertices), arrowVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);

    // Circle geometry for rotation gizmo
    const int segments = 64;
    std::vector<float> circleVertices;
    for (int i = 0; i <= segments; i++)
    {
        float angle = (float)i / (float)segments * 2.0f * 3.14159265359f;
        circleVertices.push_back(cosf(angle));
        circleVertices.push_back(sinf(angle));
        circleVertices.push_back(0.0f);
    }

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void GizmoRenderer::RenderGizmos(Actor* selectedActor, Camera* camera)
{
    if (!enabled || !selectedActor || !camera) return;

    glm::vec3 position = selectedActor->transform->position;
    glm::mat4 view = camera->view;
    glm::mat4 projection = camera->projection;

    // Disable depth testing so gizmos always render on top
    glDisable(GL_DEPTH_TEST);
    glLineWidth(3.0f);

    switch (currentMode)
    {
        case GizmoMode::Translate:
            RenderTranslateGizmo(position, view, projection);
            break;
        case GizmoMode::Rotate:
            RenderRotateGizmo(position, view, projection);
            break;
        case GizmoMode::Scale:
            RenderScaleGizmo(position, view, projection);
            break;
    }

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GizmoRenderer::RenderTranslateGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection)
{
    float gizmoSize = CalculateGizmoSize(position, view, projection, 
        glm::vec2(WindowManager::GetInstance().GetWindowWidth(), 
                  WindowManager::GetInstance().GetWindowHeight()));

    // Determine color based on drag or hover state
    auto getAxisColor = [this](GizmoAxis axis, const glm::vec3& baseColor) -> glm::vec3 {
        if (isDragging && dragAxis == axis) return glm::vec3(1, 1, 0); // Yellow when dragging
        if (!isDragging && hoveredAxis == axis) return glm::vec3(1, 0.8f, 0); // Orange when hovering
        return baseColor; // Original color
    };

    // X axis (Red)
    DrawArrow(position, glm::vec3(1, 0, 0), gizmoSize, 
              getAxisColor(GizmoAxis::X, glm::vec3(1, 0, 0)), 
              view, projection);

    // Y axis (Green)
    DrawArrow(position, glm::vec3(0, 1, 0), gizmoSize, 
              getAxisColor(GizmoAxis::Y, glm::vec3(0, 1, 0)), 
              view, projection);

    // Z axis (Blue)
    DrawArrow(position, glm::vec3(0, 0, 1), gizmoSize, 
              getAxisColor(GizmoAxis::Z, glm::vec3(0, 0, 1)), 
              view, projection);
}

void GizmoRenderer::RenderRotateGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection)
{
    float gizmoSize = CalculateGizmoSize(position, view, projection, 
        glm::vec2(WindowManager::GetInstance().GetWindowWidth(), 
                  WindowManager::GetInstance().GetWindowHeight()));

    // Determine color based on drag or hover state
    auto getAxisColor = [this](GizmoAxis axis, const glm::vec3& baseColor) -> glm::vec3 {
        if (isDragging && dragAxis == axis) return glm::vec3(1, 1, 0); // Yellow when dragging
        if (!isDragging && hoveredAxis == axis) return glm::vec3(1, 0.8f, 0); // Orange when hovering
        return baseColor; // Original color
    };

    // X circle (Red)
    DrawCircle(position, glm::vec3(1, 0, 0), gizmoSize, 
               getAxisColor(GizmoAxis::X, glm::vec3(1, 0, 0)), 
               view, projection);

    // Y circle (Green)
    DrawCircle(position, glm::vec3(0, 1, 0), gizmoSize, 
               getAxisColor(GizmoAxis::Y, glm::vec3(0, 1, 0)), 
               view, projection);

    // Z circle (Blue)
    DrawCircle(position, glm::vec3(0, 0, 1), gizmoSize, 
               getAxisColor(GizmoAxis::Z, glm::vec3(0, 0, 1)), 
               view, projection);
}

void GizmoRenderer::RenderScaleGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection)
{
    float gizmoSize = CalculateGizmoSize(position, view, projection, 
        glm::vec2(WindowManager::GetInstance().GetWindowWidth(), 
                  WindowManager::GetInstance().GetWindowHeight()));

    // Determine color based on drag or hover state
    auto getAxisColor = [this](GizmoAxis axis, const glm::vec3& baseColor) -> glm::vec3 {
        if (isDragging && dragAxis == axis) return glm::vec3(1, 1, 0); // Yellow when dragging
        if (!isDragging && hoveredAxis == axis) return glm::vec3(1, 0.8f, 0); // Orange when hovering
        return baseColor; // Original color
    };

    // Similar to translate but with cubes at the end instead of arrows
    DrawLine(position, position + glm::vec3(gizmoSize, 0, 0), 
             getAxisColor(GizmoAxis::X, glm::vec3(1, 0, 0)), 
             view, projection);

    DrawLine(position, position + glm::vec3(0, gizmoSize, 0), 
             getAxisColor(GizmoAxis::Y, glm::vec3(0, 1, 0)), 
             view, projection);

    DrawLine(position, position + glm::vec3(0, 0, gizmoSize), 
             getAxisColor(GizmoAxis::Z, glm::vec3(0, 0, 1)), 
             view, projection);
}

void GizmoRenderer::DrawArrow(const glm::vec3& start, const glm::vec3& direction, float length, 
                               const glm::vec3& color, const glm::mat4& view, const glm::mat4& projection)
{
    glUseProgram(lineShaderProgram);

    // Calculate rotation to align arrow with direction
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 normalizedDir = glm::normalize(direction);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), start);
    
    // Rotate to align with direction
    if (glm::length(glm::cross(up, normalizedDir)) > 0.001f)
    {
        float angle = acos(glm::dot(up, normalizedDir));
        glm::vec3 axis = glm::normalize(glm::cross(up, normalizedDir));
        model = glm::rotate(model, angle, axis);
    }
    else if (glm::dot(up, normalizedDir) < 0)
    {
        model = glm::rotate(model, 3.14159265359f, glm::vec3(1, 0, 0));
    }
    
    model = glm::scale(model, glm::vec3(length));

    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(lineShaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(arrowVAO);
    glDrawArrays(GL_LINES, 0, 10);
    glBindVertexArray(0);
}

void GizmoRenderer::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, 
                              const glm::mat4& view, const glm::mat4& projection)
{
    float lineVertices[] = {
        start.x, start.y, start.z,
        end.x, end.y, end.z
    };

    GLuint tempVAO, tempVBO;
    glGenVertexArrays(1, &tempVAO);
    glGenBuffers(1, &tempVBO);
    
    glBindVertexArray(tempVAO);
    glBindBuffer(GL_ARRAY_BUFFER, tempVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertices), lineVertices, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(lineShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(lineShaderProgram, "color"), 1, glm::value_ptr(color));

    glDrawArrays(GL_LINES, 0, 2);
    
    glDeleteVertexArrays(1, &tempVAO);
    glDeleteBuffers(1, &tempVBO);
}

void GizmoRenderer::DrawCircle(const glm::vec3& center, const glm::vec3& normal, float radius, 
                                const glm::vec3& color, const glm::mat4& view, const glm::mat4& projection)
{
    glUseProgram(lineShaderProgram);

    glm::vec3 up = glm::vec3(0, 0, 1);
    glm::vec3 normalizedNormal = glm::normalize(normal);
    
    glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
    
    // Rotate to align with normal
    if (glm::length(glm::cross(up, normalizedNormal)) > 0.001f)
    {
        float angle = acos(glm::dot(up, normalizedNormal));
        glm::vec3 axis = glm::normalize(glm::cross(up, normalizedNormal));
        model = glm::rotate(model, angle, axis);
    }
    else if (glm::dot(up, normalizedNormal) < 0)
    {
        model = glm::rotate(model, 3.14159265359f, glm::vec3(1, 0, 0));
    }
    
    model = glm::scale(model, glm::vec3(radius));

    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(lineShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(lineShaderProgram, "color"), 1, glm::value_ptr(color));

    glBindVertexArray(circleVAO);
    glDrawArrays(GL_LINE_LOOP, 0, 65);
    glBindVertexArray(0);
}

float GizmoRenderer::CalculateGizmoSize(const glm::vec3& position, const glm::mat4& view, 
                                         const glm::mat4& projection, const glm::vec2& screenSize)
{
    // Calculate size based on distance to camera to maintain consistent screen size
    glm::vec4 viewPos = view * glm::vec4(position, 1.0f);
    float distance = glm::length(glm::vec3(viewPos));
    
    // Size should be roughly 10% of the screen at any distance
    return distance * 0.15f;
}

GizmoAxis GizmoRenderer::HitTest(const glm::vec2& mousePos, const glm::vec2& screenSize, 
                                  const glm::mat4& view, const glm::mat4& projection, 
                                  const glm::vec3& gizmoPosition)
{
    // Simple hit testing based on screen-space distance to axis lines
    // This is a simplified version - a full implementation would use ray-axis intersection
    
    float gizmoSize = CalculateGizmoSize(gizmoPosition, view, projection, screenSize);
    
    // Project gizmo axes to screen space
    auto projectToScreen = [&](const glm::vec3& worldPos) -> glm::vec2 {
        glm::vec4 clipSpace = projection * view * glm::vec4(worldPos, 1.0f);
        glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;
        return glm::vec2((ndc.x + 1.0f) * 0.5f * screenSize.x, 
                         (1.0f - ndc.y) * 0.5f * screenSize.y);
    };

    glm::vec2 centerScreen = projectToScreen(gizmoPosition);
    glm::vec2 xAxisScreen = projectToScreen(gizmoPosition + glm::vec3(gizmoSize, 0, 0));
    glm::vec2 yAxisScreen = projectToScreen(gizmoPosition + glm::vec3(0, gizmoSize, 0));
    glm::vec2 zAxisScreen = projectToScreen(gizmoPosition + glm::vec3(0, 0, gizmoSize));

    float threshold = 15.0f; // pixels

    // Check distance to each axis line
    auto distanceToLine = [](glm::vec2 point, glm::vec2 lineStart, glm::vec2 lineEnd) -> float {
        glm::vec2 line = lineEnd - lineStart;
        float lineLength = glm::length(line);
        if (lineLength < 0.001f) return glm::length(point - lineStart);
        
        float t = glm::clamp(glm::dot(point - lineStart, line) / (lineLength * lineLength), 0.0f, 1.0f);
        glm::vec2 projection = lineStart + t * line;
        return glm::length(point - projection);
    };

    float distX = distanceToLine(mousePos, centerScreen, xAxisScreen);
    float distY = distanceToLine(mousePos, centerScreen, yAxisScreen);
    float distZ = distanceToLine(mousePos, centerScreen, zAxisScreen);

    float minDist = glm::min(distX, glm::min(distY, distZ));

    if (minDist > threshold) return GizmoAxis::None;

    if (minDist == distX) return GizmoAxis::X;
    if (minDist == distY) return GizmoAxis::Y;
    return GizmoAxis::Z;
}

void GizmoRenderer::StartDrag(GizmoAxis axis, const glm::vec2& mousePos, 
                               const glm::vec2& screenSize, const glm::mat4& view, 
                               const glm::mat4& projection, Actor* actor)
{
    if (axis == GizmoAxis::None || !actor) return;

    isDragging = true;
    dragAxis = axis;
    dragActor = actor;
    dragStartMousePos = mousePos;
    dragStartActorPos = actor->transform->position;
    dragStartActorRotation = actor->transform->eulerAngles;
    dragStartActorScale = actor->transform->scale;
    hoveredAxis = GizmoAxis::None; // Clear hover when dragging starts
    
    // Calculate initial world position for dragging
    glm::vec3 rayDir = ScreenToWorld(mousePos, screenSize, view, projection, 0.5f);
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    dragStartWorldPos = cameraPos;
    
    std::cout << "[GizmoRenderer] Started drag on " << 
        (dragAxis == GizmoAxis::X ? "X" : dragAxis == GizmoAxis::Y ? "Y" : "Z") << 
        " axis in " << 
        (currentMode == GizmoMode::Translate ? "Translate" : 
         currentMode == GizmoMode::Rotate ? "Rotate" : "Scale") << " mode" << std::endl;
}

void GizmoRenderer::UpdateDrag(const glm::vec2& mousePos, const glm::vec2& screenSize,
                                const glm::mat4& view, const glm::mat4& projection)
{
    if (!isDragging || !dragActor) return;

    // Calculate mouse delta
    glm::vec2 mouseDelta = mousePos - dragStartMousePos;
    
    // Get camera position
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    
    // Get ray direction from mouse
    glm::vec3 rayDir = ScreenToWorld(mousePos, screenSize, view, projection, 0.5f);

    switch (currentMode)
    {
        case GizmoMode::Translate:
        {
            // Project mouse movement onto the selected axis
            glm::vec3 axisDir(0.0f);
            switch (dragAxis)
            {
                case GizmoAxis::X: axisDir = glm::vec3(1, 0, 0); break;
                case GizmoAxis::Y: axisDir = glm::vec3(0, 1, 0); break;
                case GizmoAxis::Z: axisDir = glm::vec3(0, 0, 1); break;
                default: return;
            }
            
            // Create a plane perpendicular to camera view that contains the axis
            glm::vec3 cameraForward = glm::normalize(dragStartActorPos - cameraPos);
            glm::vec3 planeNormal = glm::normalize(glm::cross(axisDir, glm::cross(cameraForward, axisDir)));
            
            // Intersect ray with plane
            float denom = glm::dot(planeNormal, rayDir);
            if (std::abs(denom) > 0.0001f)
            {
                float t = glm::dot(dragStartActorPos - cameraPos, planeNormal) / denom;
                glm::vec3 intersectionPoint = cameraPos + t * rayDir;
                
                // Project intersection onto axis
                glm::vec3 projectedPoint = ProjectPointOntoLine(intersectionPoint, dragStartActorPos, axisDir);
                glm::vec3 offset = projectedPoint - dragStartActorPos;
                
                dragActor->transform->SetPosition(dragStartActorPos + offset);
            }
            break;
        }
        
        case GizmoMode::Rotate:
        {
            // Rotation based on mouse movement
            float rotationSpeed = 0.5f;
            float rotationAmount = (mouseDelta.x - mouseDelta.y) * rotationSpeed;
            
            glm::vec3 newRotation = dragStartActorRotation;
            switch (dragAxis)
            {
                case GizmoAxis::X: newRotation.x += rotationAmount; break;
                case GizmoAxis::Y: newRotation.y += rotationAmount; break;
                case GizmoAxis::Z: newRotation.z += rotationAmount; break;
                default: return;
            }
            
            dragActor->transform->SetRotation(newRotation);
            break;
        }
        
        case GizmoMode::Scale:
        {
            // Scale based on mouse movement
            float scaleSpeed = 0.01f;
            float scaleAmount = (mouseDelta.x + mouseDelta.y) * scaleSpeed;
            
            glm::vec3 newScale = dragStartActorScale;
            switch (dragAxis)
            {
                case GizmoAxis::X: newScale.x = glm::max(0.01f, dragStartActorScale.x + scaleAmount); break;
                case GizmoAxis::Y: newScale.y = glm::max(0.01f, dragStartActorScale.y + scaleAmount); break;
                case GizmoAxis::Z: newScale.z = glm::max(0.01f, dragStartActorScale.z + scaleAmount); break;
                default: return;
            }
            
            dragActor->transform->SetScale(newScale);
            break;
        }
    }
}

void GizmoRenderer::EndDrag()
{
    if (isDragging)
    {
        std::cout << "[GizmoRenderer] Ended drag operation" << std::endl;
    }
    
    isDragging = false;
    dragAxis = GizmoAxis::None;
    dragActor = nullptr;
    hoveredAxis = GizmoAxis::None; // Clear hover when drag ends
}

// ============================================================================
// High-level Mouse Interaction
// ============================================================================

void GizmoRenderer::HandleMouseDown(const glm::vec2& mousePos, const glm::vec2& screenSize,
                                   const glm::mat4& view, const glm::mat4& projection, Actor* actor)
{
    if (!actor) return;

    glm::vec3 gizmoPos = actor->transform->position;
    GizmoAxis hitAxis = HitTest(mousePos, screenSize, view, projection, gizmoPos);

    if (hitAxis != GizmoAxis::None)
    {
        StartDrag(hitAxis, mousePos, screenSize, view, projection, actor);
    }
}

void GizmoRenderer::HandleMouseMove(const glm::vec2& mousePos, const glm::vec2& screenSize,
                                   const glm::mat4& view, const glm::mat4& projection)
{
    if (isDragging)
    {
        UpdateDrag(mousePos, screenSize, view, projection);
    }
}

void GizmoRenderer::HandleMouseUp()
{
    if (isDragging)
    {
        EndDrag();
    }
}

GizmoAxis GizmoRenderer::GetHoveredAxis(const glm::vec2& mousePos, const glm::vec2& screenSize,
                                       const glm::mat4& view, const glm::mat4& projection,
                                       const glm::vec3& gizmoPosition)
{
    hoveredAxis = HitTest(mousePos, screenSize, view, projection, gizmoPosition);
    return hoveredAxis;
}

// ============================================================================
// Ray Casting and Intersection
// ============================================================================

glm::vec3 GizmoRenderer::ScreenToWorld(const glm::vec2& screenPos, const glm::vec2& screenSize,
                                      const glm::mat4& view, const glm::mat4& projection,
                                      float depth)
{
    // Convert screen coordinates to NDC
    float x = (2.0f * screenPos.x) / screenSize.x - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y) / screenSize.y;
    float z = 2.0f * depth - 1.0f;

    glm::vec4 clipCoords(x, y, z, 1.0f);

    // Convert to view space
    glm::mat4 invProjection = glm::inverse(projection);
    glm::vec4 viewCoords = invProjection * clipCoords;
    viewCoords = glm::vec4(viewCoords.x, viewCoords.y, -1.0f, 0.0f);

    // Convert to world space
    glm::mat4 invView = glm::inverse(view);
    glm::vec4 worldCoords = invView * viewCoords;

    return glm::normalize(glm::vec3(worldCoords));
}

bool GizmoRenderer::RayIntersectsArrow(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                      const glm::vec3& arrowStart, const glm::vec3& arrowDir,
                                      float arrowLength, float threshold)
{
    // Find closest point on arrow axis to ray
    glm::vec3 arrowEnd = arrowStart + arrowDir * arrowLength;
    
    // Use parametric line equations
    // Ray: P = rayOrigin + t * rayDir
    // Arrow: Q = arrowStart + s * arrowDir
    
    glm::vec3 w0 = rayOrigin - arrowStart;
    float a = glm::dot(rayDir, rayDir);
    float b = glm::dot(rayDir, arrowDir);
    float c = glm::dot(arrowDir, arrowDir);
    float d = glm::dot(rayDir, w0);
    float e = glm::dot(arrowDir, w0);
    
    float denom = a * c - b * b;
    if (std::abs(denom) < 0.0001f) return false;
    
    float s = (b * d - a * e) / denom;
    float t = (c * d - b * e) / denom;
    
    // Check if s is within arrow length
    if (s < 0.0f || s > arrowLength) return false;
    
    // Calculate closest points
    glm::vec3 pointOnRay = rayOrigin + t * rayDir;
    glm::vec3 pointOnArrow = arrowStart + s * arrowDir;
    
    // Check distance between closest points
    float distance = glm::length(pointOnRay - pointOnArrow);
    return distance < threshold;
}

bool GizmoRenderer::RayIntersectsCircle(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                       const glm::vec3& circleCenter, const glm::vec3& circleNormal,
                                       float radius, float threshold)
{
    // Intersect ray with plane containing the circle
    float denom = glm::dot(circleNormal, rayDir);
    if (std::abs(denom) < 0.0001f) return false;
    
    float t = glm::dot(circleCenter - rayOrigin, circleNormal) / denom;
    if (t < 0.0f) return false;
    
    // Get intersection point on plane
    glm::vec3 intersectionPoint = rayOrigin + t * rayDir;
    
    // Check if point is near the circle
    float distFromCenter = glm::length(intersectionPoint - circleCenter);
    return std::abs(distFromCenter - radius) < threshold;
}

glm::vec3 GizmoRenderer::ProjectPointOntoPlane(const glm::vec3& point, const glm::vec3& planeNormal,
                                               const glm::vec3& planePoint)
{
    float distance = glm::dot(point - planePoint, planeNormal);
    return point - distance * planeNormal;
}

glm::vec3 GizmoRenderer::ProjectPointOntoLine(const glm::vec3& point, const glm::vec3& lineStart,
                                              const glm::vec3& lineDir)
{
    glm::vec3 v = point - lineStart;
    float t = glm::dot(v, lineDir);
    return lineStart + t * lineDir;
}