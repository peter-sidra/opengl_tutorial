#include "indexbuffer.h"

#include "glbinding/gl/gl.h"
#include "renderer.h"

using namespace gl;

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count)
	: m_rendererID(0), m_count(count) {
	ASSERT(sizeof(unsigned int) == sizeof(GLuint));
	// This function generates a buffer and stores
	// its id in the second argument
	GLCall(glGenBuffers(1, &m_rendererID));
	// Select the buffer
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
	// Set the buffer data in vram
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_count * sizeof(unsigned int),
						data, GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(IndexBuffer &&other) {
	this->m_rendererID = other.m_rendererID;
	this->m_count = other.m_count;
	other.moved = true;
}

IndexBuffer &IndexBuffer::operator=(IndexBuffer &&other) {
	if (this == &other) {
		return *this;
	}

	// Free existing resources being held by this object
	GLCall(glDeleteBuffers(1, &m_rendererID));

	this->m_rendererID = other.m_rendererID;
	this->m_count = other.m_count;
	other.moved = true;

	return *this;
}

IndexBuffer::~IndexBuffer() {
	if (!moved)
		GLCall(glDeleteBuffers(1, &m_rendererID));
}

void IndexBuffer::bind() const {
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_rendererID));
}

void IndexBuffer::unbind() const {
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
