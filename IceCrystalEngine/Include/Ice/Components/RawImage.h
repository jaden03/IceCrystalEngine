#pragma once

#ifndef RAW_IMAGE_H

#define RAW_IMAGE_H

#include <iostream>

#include <Ice/Core/WindowManager.h>
#include <Ice/Rendering/Shader.h>
#include <Ice/Rendering/Texture.h>
#include <Ice/Core/Component.h>

#include "glad/glad.h"

class RawImage : public Component
{
public:
    RawImage(std::string texturePath);
    RawImage(std::string texturePath, std::string shaderPath);
    ~RawImage();

    Texture* texture;
    Shader* shader;

    void OverlayUpdate() override;
    
private:
    WindowManager& windowManager = WindowManager::GetInstance();
    
    static GLuint sharedVAO;
    static GLuint sharedVBO;

    static void InitializeSharedResources();
};

#endif