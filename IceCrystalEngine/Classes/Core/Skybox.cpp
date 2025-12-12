#include <Ice/Core/Skybox.h>
#include <Ice/Utils/stb_image.h>

#include <iostream>
#include <Ice/Utils/FileUtil.h>

Skybox::Skybox()
{
	SkyboxPath = FileUtil::SubstituteVariables(SkyboxPath);
	InitializeSkybox();
}

void Skybox::InitializeSkybox()
{
	// Generate the texture
	glGenTextures(1, &cubemapTextureHandle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureHandle);

	std::vector<std::string> faces
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"front.png",
		"back.png"
	};

	// Flip the image, for some reason the images in the cubemap randomly went upside down
	stbi_set_flip_vertically_on_load(false);
	
	// Load the image
	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load((SkyboxPath + "/" + faces[i]).c_str(), &width, &height, &nrChannels, 0);		if (data)
		{
			// Generate the texture
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Failed to load texture: " << SkyboxPath + "/" + faces[i] << std::endl;
		}
		// Free the image memory
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Load the model
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	shader = new Shader(FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/skybox.vert"), FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/skybox.frag"));
}



void Skybox::Render()
{
	glDepthFunc(GL_LEQUAL);

	// bind the vertex array object
	glBindVertexArray(skyboxVAO);

	// bind the texture
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureHandle);

	shader->Use();
	
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);
	
	// camera stuff
	if (sceneManager.mainCamera != nullptr)
	{
		Camera* mainCamera = sceneManager.mainCamera;
		view = glm::lookAt(mainCamera->transform->position, mainCamera->transform->position + mainCamera->transform->forward, mainCamera->transform->up);
		view = glm::mat4(glm::mat3(view));

		// Prevent division by zero - use 16:9 as fallback if window not initialized
		float aspectRatio = 16.0f / 9.0f;
		if (windowManager.windowWidth > 0 && windowManager.windowHeight > 0)
		{
			aspectRatio = (float)windowManager.windowWidth / (float)windowManager.windowHeight;
		}

		projection = glm::perspective(glm::radians(sceneManager.mainCamera->fieldOfView), aspectRatio, sceneManager.mainCamera->nearClippingPlane, sceneManager.mainCamera->farClippingPlane);
	}
	else
	{
		// Prevent division by zero - use 16:9 as fallback if window not initialized
		float aspectRatio = 16.0f / 9.0f;
		if (windowManager.windowWidth > 0 && windowManager.windowHeight > 0)
		{
			aspectRatio = (float)windowManager.windowWidth / (float)windowManager.windowHeight;
		}

		projection = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 10000.0f);
	}

	shader->setMat4("projection", projection);
	shader->setMat4("view", view);

	// draw the mesh
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// unbind the vertex array object
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
}