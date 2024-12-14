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
    RawImage(std::string path);
    ~RawImage();

    Material* material;

    void OverlayUpdate() override;
    
private:
    WindowManager& windowManager = WindowManager::GetInstance();
    
    static GLuint sharedVAO;
    static GLuint sharedVBO;

    static void InitializeSharedResources();
};

#endif