#define STB_IMAGE_IMPLEMENTATION
#include <Ice/Rendering/Texture.h>
#include <Ice/Utils/stb_image.h>

#include <iostream>
#include <Ice/Utils/FileUtil.h>

Texture::Texture()
{
	InitializeTexture();
}

Texture::Texture(std::string texturePath)
{
	TexturePath = texturePath;
	InitializeTexture();
}



void Texture::InitializeTexture()
{
	// Parse the path
	std::string path = FileUtil::SubstituteVariables(TexturePath);
	TexturePath = path;

	// Generate the texture
	glGenTextures(1, &Handle);
	glBindTexture(GL_TEXTURE_2D, Handle);

	// Set the texture wrapping/filtering options (on the currently bound texture object)
	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// set flip on load to true
	stbi_set_flip_vertically_on_load(true);
	
	// Load the image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(TexturePath.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		// Generate the texture
		GLenum format;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		else {
			std::cout << "Unsupported number of channels: " << nrChannels << std::endl;
			stbi_image_free(data);
			return;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture: " << TexturePath << std::endl;
	}
	// Free the image memory
	stbi_image_free(data);
}