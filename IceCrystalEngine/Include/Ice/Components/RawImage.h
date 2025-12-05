#pragma once

#ifndef RAW_IMAGE_H

#define RAW_IMAGE_H

#include <iostream>

#include <Ice/Core/WindowManager.h>
#include <Ice/Rendering/Shader.h>
#include <Ice/Rendering/Texture.h>
#include <Ice/Core/Component.h>

#include "glad/glad.h"

enum class RawImageSourceType
{
    EngineTexture,
    RawHandle
};

class RawImage : public Component
{
public:
    RawImage(std::string texturePath);
    RawImage(std::string texturePath, std::string shaderPath);
    RawImage(unsigned int textureHandle);
    RawImage(unsigned int textureHandle, std::string shaderPath);
    ~RawImage();
    
    Texture* texture;
    Shader* shader;

    GLuint rawHandle = 0;
    RawImageSourceType sourceType = RawImageSourceType::EngineTexture;

    void OverlayUpdate() override;
    
private:
    WindowManager& windowManager = WindowManager::GetInstance();
    
    static GLuint sharedVAO;
    static GLuint sharedVBO;

    static void InitializeSharedResources();
};

#endif