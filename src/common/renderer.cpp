#include "renderer.h"

#include <iostream>

void GLClearErrors() {
	gl::GLenum error_code;
	do {
		error_code = gl::glGetError();
	} while (error_code != gl::GL_NO_ERROR);
}

void GLPrintError(gl::GLenum error_code) {
	switch (error_code) {
	case gl::GLenum::GL_INVALID_ENUM:
		std::cerr << "[GLError] GL_INVALID_ENUM" << std::endl;
		break;
	case gl::GLenum::GL_INVALID_VALUE:
		std::cerr << "[GLError] GL_INVALID_VALUE" << std::endl;
		break;
	case gl::GLenum::GL_INVALID_OPERATION:
		std::cerr << "[GLError] GL_INVALID_OPERATION" << std::endl;
		break;
	case gl::GLenum::GL_INVALID_FRAMEBUFFER_OPERATION:
		std::cerr << "[GLError] GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
		break;
	case gl::GLenum::GL_OUT_OF_MEMORY:
		std::cerr << "[GLError] GL_OUT_OF_MEMORY" << std::endl;
		break;
	case gl::GLenum::GL_STACK_UNDERFLOW:
		std::cerr << "[GLError] GL_STACK_UNDERFLOW" << std::endl;
		break;
	case gl::GLenum::GL_STACK_OVERFLOW:
		std::cerr << "[GLError] GL_STACK_OVERFLOW" << std::endl;
		break;
	default:
		break;
	}
}

void GLCheckErrors() {
	gl::GLenum error_code;
	do {
		error_code = gl::glGetError();
		GLPrintError(error_code);
		// std::cerr << "GLError! Code: " << erorr_code << std::endl;
	} while (error_code != gl::GL_NO_ERROR);
}

bool GLLogCall(const char *function, const char *file, int line) {
	gl::GLenum error_code = gl::glGetError();

	if (error_code == gl::GLenum::GL_NO_ERROR)
		return true;

	GLPrintError(error_code);
	std::cerr << function << " " << file << ":" << line << std::endl;

	return false;
}