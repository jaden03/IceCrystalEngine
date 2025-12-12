#include <Ice/Rendering/PostProcessor.h>

#include <Ice/Core/WindowManager.h>
#include <Ice/Core/LightingManager.h>
#include <Ice/Utils/FileUtil.h>

#include <Ice/Rendering/Shader.h>

#include <iostream>

PostProcessor::PostProcessor()
{
	// Multisampled FBO
	glGenFramebuffers(1, &multisampledFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);

	glGenTextures(3, multisampledColorBuffers);

	for (unsigned int i = 0; i < 3; i++)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampledColorBuffers[i]);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB16F, windowManager.windowWidth, windowManager.windowHeight, GL_TRUE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, multisampledColorBuffers[i], 0);
	}

	unsigned int multisampledAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, multisampledAttachments);

	glGenRenderbuffers(1, &multisampledRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, multisampledRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, windowManager.windowWidth, windowManager.windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, multisampledRBO);
	
	
	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::POSTPROCESSOR: Failed to initialize multisampled FBO" << std::endl;
	// ------------------------------- \\



	// HDR FBO
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	glGenTextures(3, colorBuffers);
	
	for (unsigned int i = 0; i < 3; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}

	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowManager.windowWidth, windowManager.windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	
	
	// Check if the framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	// ------------------------------- \\

	
	
	lastScreenHeight = windowManager.windowHeight;
	lastScreenWidth = windowManager.windowWidth;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create the shader
	hdrShader = new Shader(FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/hdr.vert"), FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/hdr.frag"));


	// Bloom
	glGenFramebuffers(2, bloomPingpongFBO);
	glGenTextures(2, bloomPingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloomPingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, bloomPingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPingpongBuffer[i], 0);
	}

	// Blur
	glGenFramebuffers(2, blurPingpongFBO);
	glGenTextures(2, blurPingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, blurPingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, blurPingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurPingpongBuffer[i], 0);
	}
	
	blurShader = new Shader(FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/blur.vert"), FileUtil::SubstituteVariables("{ENGINE_ASSET_DIR}Shaders/blur.frag"));
}


void PostProcessor::Render()
{
	// blit multisampledFBO to hdrFBO
	for (int i = 0; i < 3; i++)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO);
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
		glBlitFramebuffer(0, 0, windowManager.windowWidth, windowManager.windowHeight, 0, 0, windowManager.windowWidth, windowManager.windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	// get the hovered actor color
	float* buffer = new float[3];
	double mouseX, mouseY;
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glfwGetCursorPos(windowManager.window, &mouseX, &mouseY);
	glReadPixels(mouseX, windowManager.windowHeight - mouseY, 1, 1, GL_RGB, GL_FLOAT, buffer);
	
	glm::vec3 decodedColor = glm::vec3(buffer[0], buffer[1], buffer[2]);
	delete[] buffer;
	
	decodedColor *= 255.0f;

	decodedColor.x = round(decodedColor.x);
	decodedColor.y = round(decodedColor.y);
	decodedColor.z = round(decodedColor.z);

	hoveredActorColor = decodedColor;
	
	if (lastScreenHeight != windowManager.windowHeight || lastScreenWidth != windowManager.windowWidth)
	{
		// HDR Color Buffers
		for (unsigned int i = 0; i < 3; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		}
		// Bloom and Blur Buffers
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, bloomPingpongBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);

			glBindTexture(GL_TEXTURE_2D, blurPingpongBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowManager.windowWidth, windowManager.windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
		}
		// Multisampled Color Buffers
		for (unsigned int i = 0; i < 3; i++)
		{
			glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampledColorBuffers[i]);
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB16F, windowManager.windowWidth, windowManager.windowHeight, GL_TRUE);
		}
		// Depth Buffers
		glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowManager.windowWidth, windowManager.windowHeight);
		
		glBindRenderbuffer(GL_RENDERBUFFER, multisampledRBO);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, windowManager.windowWidth, windowManager.windowHeight);
		
		
		lastScreenHeight = windowManager.windowHeight;
		lastScreenWidth = windowManager.windowWidth;
	}

	
	// bloom
	bool bloomBlurHorizontal = true, bloomBlurFirstIteration = true;
	if (bloom)
	{
		int amount = 2;
		blurShader->Use();
		for (unsigned int i = 0; i < amount * 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, bloomPingpongFBO[bloomBlurHorizontal]);
			blurShader->setInt("horizontal", bloomBlurHorizontal);
			glActiveTexture(GL_TEXTURE0);
			// if its the first iteration, it will use the colorBuffer from the hdrFBO
			glBindTexture(GL_TEXTURE_2D, bloomBlurFirstIteration ? colorBuffers[1] : bloomPingpongBuffer[!bloomBlurHorizontal]);
			RenderFullscreenQuad();
			bloomBlurHorizontal = !bloomBlurHorizontal;
			bloomBlurFirstIteration = false;
		}
	}

	// blur
	bool blurBlurHorizontal = true, blurBlurFirstIteration = true;
	if (blurIterations > 0)
	{
		blurShader->Use();
		// it does it twice as much as the set iterations because each iteration it swaps between vertical and horizontal
		// basically if it were to be an odd number, it would be blurred in one direction more than the other
		for (unsigned int i = 0; i < blurIterations * 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurPingpongFBO[blurBlurHorizontal]);
			blurShader->setInt("horizontal", blurBlurHorizontal);
			glActiveTexture(GL_TEXTURE0);
			// if its the first iteration, it will use the colorBuffer from the hdrFBO
			glBindTexture(GL_TEXTURE_2D, blurBlurFirstIteration ? colorBuffers[0] : blurPingpongBuffer[!blurBlurHorizontal]);
			RenderFullscreenQuad();
			blurBlurHorizontal = !blurBlurHorizontal;
			blurBlurFirstIteration = false;
		}
	}


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	hdrShader->Use();
	
	glActiveTexture(GL_TEXTURE0);
	if (blurIterations > 0)
		glBindTexture(GL_TEXTURE_2D, blurPingpongBuffer[!blurBlurHorizontal]);
	else
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	hdrShader->setInt("colorBuffer", 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, bloomPingpongBuffer[!bloomBlurHorizontal]); // always the last one
	hdrShader->setInt("bloomBuffer", 1);
	
	hdrShader->setFloat("exposure", exposure);
	hdrShader->setBool("bloom", bloom);
	
	RenderFullscreenQuad();
}


void PostProcessor::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, multisampledFBO);
}

// Rendering the fullscreen quad
unsigned int quadVAO = 0;
unsigned int quadVBO;
void PostProcessor::RenderFullscreenQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}