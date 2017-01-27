#include "FrameBufferObject.h"
#include <iostream>
 
FrameBufferObject::FrameBufferObject()
{
	/// CODE HERE ////////////////////////////////////////////////////////////

}

FrameBufferObject::~FrameBufferObject()
{
	destroy();
}

void FrameBufferObject::createFrameBuffer(unsigned int fboWidth, unsigned int fboHeight, unsigned int numColourBuffers, bool useDepth)
{
	/// CODE HERE ////////////////////////////////////////////////////////////

}

void FrameBufferObject::bindFrameBufferForDrawing()
{
	/// CODE HERE ////////////////////////////////////////////////////////////

}

void FrameBufferObject::unbindFrameBuffer(int backBufferWidth, int backBufferHeight)
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}

void FrameBufferObject::clearFrameBuffer(glm::vec4 clearColour)
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}

void FrameBufferObject::bindTextureForSampling(int textureIndex, GLenum textureUnit)
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}

void FrameBufferObject::unbindTexture(GLenum textureUnit)
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}

void FrameBufferObject::destroy()
{
	/// CODE HERE ////////////////////////////////////////////////////////////
}
