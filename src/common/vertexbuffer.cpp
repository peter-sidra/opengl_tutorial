#include "vertexbuffer.h"

#include "glbinding/gl/gl.h"
#include "renderer.h"

using namespace gl;

VertexBuffer::VertexBuffer(const void *data, unsigned int size)
	: m_rendererID(0) {
	// This function generates a buffer and stores
	// its id in the second argument
	GLCall(glGenBuffers(1, &m_rendererID));
	// Select the buffer
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
	// Set the buffer data in vram
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(VertexBuffer &&other) {
	this->m_rendererID = other.m_rendererID;
	other.moved = true;
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) {
	if (this == &other) {
		return *this;
	}

	// Free existing resources being held by this object
	GLCall(glDeleteBuffers(1, &m_rendererID));

	this->m_rendererID = other.m_rendererID;
	other.moved = true;

	return *this;
}

VertexBuffer::~VertexBuffer() {
	if (!moved)
		GLCall(glDeleteBuffers(1, &m_rendererID));
}

void VertexBuffer::bind() const {
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_rendererID));
}

void VertexBuffer::unbind() const {
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
