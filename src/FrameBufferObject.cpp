#include "FrameBufferObject.h"
#include <iostream>
 
FrameBufferObject::FrameBufferObject()
{
	depthTexHandle = 0;

	numColourTex = 0;
	memset(colourTexHandles, 0, MAX_BUFFERS);

	memset(bufferAttachments, 0, MAX_BUFFERS);
	numBuffers = 0;

}

FrameBufferObject::~FrameBufferObject()
{
	destroy();
}

void FrameBufferObject::createFrameBuffer(unsigned int fboWidth, unsigned int fboHeight, unsigned int numColourBuffers, bool useDepth)
{
	width = fboWidth; // should ensure that fbo width and height are >= 0;
	height = fboHeight;
	numColourTex = numColourBuffers;

	if (numColourTex > MAX_BUFFERS) {
		std::cout << "Attempting to create " << numColourTex << " buffers. Max is " << MAX_BUFFERS << std::endl;
		numColourTex = MAX_BUFFERS;

	}

	glGenFramebuffers(1, &handle); // create the frame buffer object

	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	glGenTextures(numColourTex, colourTexHandles);

	for (int i = 0; i < numColourTex; i++)
	{
		// need to bind before we allocate memory
		glBindTexture(GL_TEXTURE_2D, colourTexHandles[i]);

		// allocate memory but don't pass to the CPU
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colourTexHandles[i], 0);

		bufferAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
		numBuffers++;
	}

	if (useDepth) {
		glGenTextures(1, &depthTexHandle);

		glBindTexture(GL_TEXTURE_2D, depthTexHandle);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexHandle, 0);
	}

	glDrawBuffers(numBuffers, bufferAttachments);

	GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Error Creating FBO" << fboStatus << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind - use the backbuffer
}

void FrameBufferObject::bindFrameBufferForDrawing()
{
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
	glViewport(0, 0, width, height);

}

void FrameBufferObject::unbindFrameBuffer(int backBufferWidth, int backBufferHeight)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, backBufferWidth, backBufferHeight);

}

void FrameBufferObject::clearFrameBuffer(glm::vec4 clearColour)
{
	glClearColor(clearColour.x, clearColour.y, clearColour.z, clearColour.w);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void FrameBufferObject::bindTextureForSampling(int textureIndex, GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, colourTexHandles[textureIndex]);
}

void FrameBufferObject::unbindTexture(GLenum textureUnit)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void FrameBufferObject::destroy()
{
	// free up all texures allocated 
	if (numColourTex > 0) {
		glDeleteTextures(numColourTex, colourTexHandles);
		memset(colourTexHandles, 0, MAX_BUFFERS);
		numColourTex = 0;
	}

	if (depthTexHandle > 0) {
		glDeleteTextures(1, &depthTexHandle);
		depthTexHandle = 0;
	}

	if (handle) {
		glDeleteFramebuffers(1, &handle);
	}
}
