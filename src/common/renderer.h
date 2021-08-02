#pragma once

#include "debugbreak.h"
#include <glbinding/gl/gl.h>


#define ASSERT(x)                                                              \
	if (!(x))                                                                  \
	debug_break()

#ifndef NDEBUG
#define GLCall(x)                                                              \
	GLClearErrors();                                                           \
	x;                                                                         \
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#define GLCallV(x)                                                             \
	[&]() {                                                                    \
		GLClearErrors();                                                       \
		auto retVal = x;                                                       \
		ASSERT(GLLogCall(#x, __FILE__, __LINE__));                             \
		return retVal;                                                         \
	}()
#else
#define GLCallV(x) x
#define GLCall(x) x
#endif

void GLClearErrors();

void GLPrintError(gl::GLenum error_code);

void GLCheckErrors();

bool GLLogCall(const char *function, const char *file, int line);