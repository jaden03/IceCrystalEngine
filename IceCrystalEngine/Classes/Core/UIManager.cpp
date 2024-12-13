#include <Ice/Core/UIManager.h>

#include <glad/glad.h>
#include <Ice/Utils/FileUtil.h>

#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/transform.hpp"

UIManager::UIManager()
{
    material = new Material(FileUtil::AssetDir + "Materials/ui.mat");
}


unsigned int uiManagerQuadVAO = 0;
unsigned int uiManagerQuadVBO;
void UIManager::RenderTest(glm::vec2 position, glm::vec2 size)
{
    if (uiManagerQuadVAO == 0)
    {
        constexpr float quadVertices[] = {
            // Positions   // Texture Coords
            -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 0.0f, // Bottom-right
            -0.5f,  0.5f,  0.0f, 1.0f, // Top-left
             0.5f,  0.5f,  1.0f, 1.0f  // Top-right
        };
        // setup plane VAO
        glGenVertexArrays(1, &uiManagerQuadVAO);
        glGenBuffers(1, &uiManagerQuadVBO);
        glBindVertexArray(uiManagerQuadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, uiManagerQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowManager.windowWidth), static_cast<float>(windowManager.windowHeight), 0.0f, -1.0f, 1.0f);

    material->shader->Use();
    material->shader->setMat4("model", model);
    material->shader->setMat4("projection", projection);

    glBindVertexArray(uiManagerQuadVAO);
    glBindTexture(GL_TEXTURE_2D, material->texture->Handle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}