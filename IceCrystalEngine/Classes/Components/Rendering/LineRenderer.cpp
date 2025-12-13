#include <Ice/Components/Rendering/LineRenderer.h>

#include <glad/glad.h>
#include <Ice/Utils/MathUtils.h>

#include <Ice/Managers/RendererManager.h>

struct LineVertex {
    glm::vec3 pos;
    glm::vec3 color;
};

void LineRenderer::Init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)offsetof(LineVertex, color));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void LineRenderer::Update()
{
    if (points.size() < 2) return;

    // Build vertex data
    std::vector<LineVertex> verts;
    verts.reserve(points.size());

    for (size_t i = 0; i < points.size(); i++) {
        float t = (float)i / (points.size() - 1);
        glm::vec3 c = MathUtils::Lerp(colorStart, colorEnd, t);
        
        glm::vec3 worldPos;
        if (worldSpace) {
            worldPos = points[i];
        } else {
            // Transform point by the transform of the actor
            worldPos = transform->position 
                     + transform->Right() * points[i].x
                     + transform->Up() * points[i].y
                     + transform->Forward() * points[i].z;
        }
        
        verts.push_back({worldPos, c});
    }

    // Upload
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(LineVertex), verts.data(), GL_DYNAMIC_DRAW);

    // Draw
    RendererManager::GetInstance().lineRendererShader->Use();
    glBindVertexArray(vao);
    glLineWidth(width);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)verts.size());
    glBindVertexArray(0);
}

LineRenderer::~LineRenderer()
{
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
