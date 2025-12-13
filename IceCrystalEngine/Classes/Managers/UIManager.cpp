#include <Ice/Core/UIManager.h>

#include <iostream>
#include <glad/glad.h>
#include <Ice/Utils/FileUtil.h>

#include "glm/gtc/type_ptr.inl"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/transform.hpp"

UIManager::UIManager()
{
    constexpr float quadVertices[] = {
        // Positions   // Texture Coords
        -0.5f, -0.5f,  0.0f, 1.0f, // Bottom-left
         0.5f, -0.5f,  1.0f, 1.0f, // Bottom-right
        -0.5f,  0.5f,  0.0f, 0.0f, // Top-left
         0.5f,  0.5f,  1.0f, 0.0f  // Top-right
    };
    // setup plane VAO
    glGenVertexArrays(1, &uiManagerVAO);
    glGenBuffers(1, &uiManagerVBO);
    glBindVertexArray(uiManagerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiManagerVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    material = new Material(FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Materials/ui.mat"));
}


void UIManager::RenderTest(glm::vec2 position, glm::vec2 size)
{
    position.x += size.x * 0.5f;
    position.y += size.y * 0.5f;
    
    glBindVertexArray(uiManagerVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->texture->Handle);
    GLint textureLocation = glGetUniformLocation(material->shader->Handle, "fragTexture");
    glUniform1i(textureLocation, 0);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    glm::mat4 projection = glm::ortho(0.0f, (float)windowManager.windowWidth, (float)windowManager.windowHeight, 0.0f, -1.0f, 1.0f);
    
    material->shader->Use();
    material->shader->setMat4("model", model);
    material->shader->setMat4("projection", projection);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}