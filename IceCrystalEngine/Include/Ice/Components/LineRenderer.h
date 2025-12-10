#pragma once

#include <Ice/Core/Component.h>
#include <Ice/Rendering/Material.h>

class LineRenderer : public Component
{
public:
    LineRenderer(std::vector<glm::vec3> points) : points(points) {Init();}
    LineRenderer(std::vector<glm::vec3> points, glm::vec3 color) : points(points), colorStart(color), colorEnd(color) {Init();}
    LineRenderer(std::vector<glm::vec3> points, glm::vec3 colorStart, glm::vec3 colorEnd) : points(points), colorStart(colorStart), colorEnd(colorEnd) {Init();}

    virtual ~LineRenderer();
    
    void Update() override;
    
    std::vector<glm::vec3> points;
    glm::vec3 colorStart = { 1.0f, 1.0f, 1.0f };
    glm::vec3 colorEnd = glm::vec3(1.0f, 1.0f, 1.0f);

    float width = 1.0f;
    bool worldSpace = true;
private:
    void Init();
    
    GLuint vao;
    GLuint vbo;
};
