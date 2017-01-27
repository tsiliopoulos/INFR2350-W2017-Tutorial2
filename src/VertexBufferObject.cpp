#include "VertexBufferObject.h"
#include <iostream>

VertexBufferObject::VertexBufferObject()
{
	vaoHandle = 0;
}

VertexBufferObject::~VertexBufferObject()
{
	destroy();
}

int VertexBufferObject::addAttributeArray(AttributeDescriptor attrib)
{
	attributeDescriptors.push_back(attrib);
	return 1;
}

void VertexBufferObject::createVBO()
{
	if (vaoHandle)
	{
		destroy();
	}

	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);

	unsigned int numBuffers = attributeDescriptors.size();
	vboHandles.resize(numBuffers);

	glGenBuffers(numBuffers, &vboHandles[0]);

	for (int i = 0; i < numBuffers; i++)
	{
		AttributeDescriptor* attrib = &attributeDescriptors[i];
		
		glEnableVertexAttribArray(attrib->attributeLocation);
		glBindBuffer(GL_ARRAY_BUFFER, vboHandles[i]);
		glBufferData(GL_ARRAY_BUFFER, attrib->numElements * attrib->elementSize,
			attrib->data, GL_STATIC_DRAW);

		glVertexAttribPointer(attrib->attributeLocation, attrib->numElementsPerAttrib,
			attrib->elementType, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindVertexArray(0);
}

void VertexBufferObject::draw()
{
	if (vaoHandle)
	{
		glBindVertexArray(vaoHandle);
		// better way would be to just store the num of vertices
		glDrawArrays(GL_TRIANGLES, 0, 
			attributeDescriptors[0].numElements / attributeDescriptors[0].numElementsPerAttrib);
	}
}

void VertexBufferObject::destroy()
{
	if (vaoHandle)
	{
		glDeleteVertexArrays(1, &vaoHandle);
		glDeleteBuffers(vboHandles.size(), &vboHandles[0]);
	}

	vboHandles.clear();
	attributeDescriptors.clear();
}


