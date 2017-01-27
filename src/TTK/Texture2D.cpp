#include "GLEW/glew.h"
#include "TTK/Texture2D.h"
#include "IL/ilut.h"

TTK::Texture2D::Texture2D()
{
	texWidth = texHeight = 0;
}


TTK::Texture2D::Texture2D(std::string filename)
{
	texWidth = texHeight = 0;
	loadTexture(filename);
}

TTK::Texture2D::~Texture2D()
{
	glDeleteTextures(1, &texID);
}

int TTK::Texture2D::width()
{
	return texWidth;
}

int TTK::Texture2D::height()
{
	return texHeight;
}

void TTK::Texture2D::loadTexture(std::string filename, bool createGLTexture, bool flip)
{
	glEnable(GL_TEXTURE_2D);

	ilGenImages(1, &texID);
	ilBindImage(texID);
	ilEnable(IL_ORIGIN_SET);

	if (flip)
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	else
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

	ILboolean ret = ilLoadImage(filename.c_str()); // location relative to executable
	texWidth = ilGetInteger(IL_IMAGE_WIDTH);
	texHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	dataType = ilGetInteger(IL_IMAGE_TYPE);
	pixelFormat = ilGetInteger(IL_IMAGE_FORMAT);

	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

	dataPtr = ilGetData();

	if (createGLTexture)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, pixelFormat, dataType, dataPtr);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	ILenum Error;
	while ((Error = ilGetError()) != IL_NO_ERROR)
	{
		printf("Texture Loading Error:\t%d: %s\n", Error, iluErrorString(Error));
	}	
}

void TTK::Texture2D::bind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
/*	glEnable(GL_TEXTURE_2D);*/

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, texID);
}

void TTK::Texture2D::unbind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}

unsigned int TTK::Texture2D::id()
{
	return texID;
}

unsigned char* TTK::Texture2D::data()
{
	return dataPtr;
}

int TTK::Texture2D::type()
{
	return dataType;
}

int TTK::Texture2D::format()
{
	return pixelFormat; 
}
