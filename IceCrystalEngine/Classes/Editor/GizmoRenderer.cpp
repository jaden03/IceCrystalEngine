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

    // X axis (Red)
    DrawArrow(position, glm::vec3(1, 0, 0), gizmoSize, 
              dragAxis == GizmoAxis::X ? glm::vec3(1, 1, 0) : glm::vec3(1, 0, 0), 
              view, projection);

    // Y axis (Green)
    DrawArrow(position, glm::vec3(0, 1, 0), gizmoSize, 
              dragAxis == GizmoAxis::Y ? glm::vec3(1, 1, 0) : glm::vec3(0, 1, 0), 
              view, projection);

    // Z axis (Blue)
    DrawArrow(position, glm::vec3(0, 0, 1), gizmoSize, 
              dragAxis == GizmoAxis::Z ? glm::vec3(1, 1, 0) : glm::vec3(0, 0, 1), 
              view, projection);
}

void GizmoRenderer::RenderRotateGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection)
{
    float gizmoSize = CalculateGizmoSize(position, view, projection, 
        glm::vec2(WindowManager::GetInstance().GetWindowWidth(), 
                  WindowManager::GetInstance().GetWindowHeight()));

    // X circle (Red)
    DrawCircle(position, glm::vec3(1, 0, 0), gizmoSize, 
               dragAxis == GizmoAxis::X ? glm::vec3(1, 1, 0) : glm::vec3(1, 0, 0), 
               view, projection);

    // Y circle (Green)
    DrawCircle(position, glm::vec3(0, 1, 0), gizmoSize, 
               dragAxis == GizmoAxis::Y ? glm::vec3(1, 1, 0) : glm::vec3(0, 1, 0), 
               view, projection);

    // Z circle (Blue)
    DrawCircle(position, glm::vec3(0, 0, 1), gizmoSize, 
               dragAxis == GizmoAxis::Z ? glm::vec3(1, 1, 0) : glm::vec3(0, 0, 1), 
               view, projection);
}

void GizmoRenderer::RenderScaleGizmo(const glm::vec3& position, const glm::mat4& view, const glm::mat4& projection)
{
    float gizmoSize = CalculateGizmoSize(position, view, projection, 
        glm::vec2(WindowManager::GetInstance().GetWindowWidth(), 
                  WindowManager::GetInstance().GetWindowHeight()));

    // Similar to translate but with cubes at the end instead of arrows
    DrawLine(position, position + glm::vec3(gizmoSize, 0, 0), 
             dragAxis == GizmoAxis::X ? glm::vec3(1, 1, 0) : glm::vec3(1, 0, 0), 
             view, projection);

    DrawLine(position, position + glm::vec3(0, gizmoSize, 0), 
             dragAxis == GizmoAxis::Y ? glm::vec3(1, 1, 0) : glm::vec3(0, 1, 0), 
             view, projection);

    DrawLine(position, position + glm::vec3(0, 0, gizmoSize), 
             dragAxis == GizmoAxis::Z ? glm::vec3(1, 1, 0) : glm::vec3(0, 0, 1), 
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
}

void GizmoRenderer::UpdateDrag(const glm::vec2& mousePos, const glm::vec2& screenSize,
                                const glm::mat4& view, const glm::mat4& projection)
{
    if (!isDragging || !dragActor) return;

    // Calculate mouse delta
    glm::vec2 mouseDelta = mousePos - dragStartMousePos;

    // Simple drag implementation - move along the selected axis based on mouse movement
    glm::vec3 offset(0.0f);
    float sensitivity = 0.01f;

    switch (dragAxis)
    {
        case GizmoAxis::X:
            offset.x = mouseDelta.x * sensitivity;
            break;
        case GizmoAxis::Y:
            offset.y = -mouseDelta.y * sensitivity;
            break;
        case GizmoAxis::Z:
            offset.z = (mouseDelta.x + mouseDelta.y) * sensitivity * 0.5f;
            break;
        default:
            break;
    }

    dragActor->transform->SetPosition(dragStartActorPos + offset);
}

void GizmoRenderer::EndDrag()
{
    isDragging = false;
    dragAxis = GizmoAxis::None;
    dragActor = nullptr;
}