#pragma once

#ifndef TEXTURE_H

#define TEXTURE_H

#include <string>

class Texture
{

	void InitializeTexture();
	
public:

	std::string TexturePath = "{ASSET_DIR}Textures/Blank.png";
	unsigned int Handle;

	Texture();
	Texture(std::string texturePath);

	void Bind();

};


#endif