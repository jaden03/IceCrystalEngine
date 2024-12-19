#pragma once

#ifndef RAW_IMAGE_H

#define RAW_IMAGE_H

#include <iostream>

#include <Ice/Core/WindowManager.h>
#include <Ice/Rendering/Material.h>
#include <Ice/Core/Component.h>

#include "glad/glad.h"

class RawImage : public Component
{
public:
    RawImage(std::string texturePath);
    RawImage(std::string texturePath, std::string shaderPath);
    ~RawImage();

    Shader* shader;
    Texture* texture;

    void OverlayUpdate() override;
    
private:
    WindowManager& windowManager = WindowManager::GetInstance();
    
    static GLuint sharedVAO;
    static GLuint sharedVBO;

    static void InitializeSharedResources();
};

#endif