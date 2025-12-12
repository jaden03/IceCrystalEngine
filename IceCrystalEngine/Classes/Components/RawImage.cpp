#include <Ice/Components/RawImage.h>

#include "glad/glad.h"

#include <Ice/Utils/FileUtil.h>

#include <utility>

#include "glm/gtx/string_cast.hpp"

GLuint RawImage::sharedVAO = 0;
GLuint RawImage::sharedVBO = 0;

RawImage::RawImage(std::string texturePath)
{
    InitializeSharedResources();
    shader = new Shader(FileUtil::EngineAssetDir + "Shaders/ui.vert", FileUtil::EngineAssetDir + "Shaders/ui.frag");
    texture = new Texture(texturePath);
}
RawImage::RawImage(std::string texturePath, std::string shaderPath)
{
    InitializeSharedResources();
    shader = new Shader(shaderPath + ".vert", shaderPath + ".frag");
    texture = new Texture(texturePath);
}

RawImage::RawImage(unsigned int textureHandle)
{
    InitializeSharedResources();
    shader = new Shader(FileUtil::EngineAssetDir + "Shaders/ui.vert", FileUtil::EngineAssetDir + "Shaders/ui.frag");
    sourceType = RawImageSourceType::RawHandle;
    rawHandle = textureHandle;
}
RawImage::RawImage(unsigned int textureHandle, std::string shaderPath)
{
    InitializeSharedResources();
    shader = new Shader(shaderPath + ".vert", shaderPath + ".frag");
    sourceType = RawImageSourceType::RawHandle;
    rawHandle = textureHandle;
}

RawImage::~RawImage()
{
    glDeleteVertexArrays(1, &sharedVAO);
    glDeleteBuffers(1, &sharedVBO);
}

void RawImage::InitializeSharedResources()
{
    if (sharedVAO == 0 && sharedVBO == 0)
    {
        constexpr float quadVertices[] = {
            // Positions   // Texture Coords
            -0.5f, -0.5f,  0.0f, 1.0f, // Bottom-left
             0.5f, -0.5f,  1.0f, 1.0f, // Bottom-right
            -0.5f,  0.5f,  0.0f, 0.0f, // Top-left
             0.5f,  0.5f,  1.0f, 0.0f  // Top-right
        };
        
        glGenVertexArrays(1, &sharedVAO);
        glGenBuffers(1, &sharedVBO);
        glBindVertexArray(sharedVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sharedVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
}

void RawImage::OverlayUpdate()
{
    if (!enabled) return;
    
    float posX = transform->position.x + (transform->scale.x * .5f);
    float posY = transform->position.y + (transform->scale.y * .5f);
    glm::vec3 pos = glm::vec3(posX, posY, transform->position.z);
    
    glActiveTexture(GL_TEXTURE0);

    if (sourceType == RawImageSourceType::RawHandle)
        glBindTexture(GL_TEXTURE_2D, rawHandle);
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture->Handle);
    }
    
    GLint textureLocation = glGetUniformLocation(shader->Handle, "fragTexture");
    glUniform1i(textureLocation, 0);
    

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, transform->scale);

    glm::mat4 projection = glm::ortho(0.0f, (float)windowManager.windowWidth, (float)windowManager.windowHeight, 0.0f, -1.0f, 1.0f);

    shader->Use();
    shader->setMat4("model", model);
    shader->setMat4("projection", projection);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindVertexArray(sharedVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}