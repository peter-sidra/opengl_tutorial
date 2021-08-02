// clang-format off
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// clang-format on
#include "debugbreak.h"
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

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

// Documentation website: docs.gl

// Summary: with vertex arrays the way that we draw things changes from
// bind(program) -> bind(vertex_buffer) -> setup_vertex_layout() ->
// bind(index_buffer) -> draw()
// to
// bind(program) -> bind(vertex_array) -> bind(index_buffer) -> draw()
// so basically binding the vertex buffer and setting up its layout becomes
// just binding the vertex array object

// Note: vertex array objects are mandatory, they are created by default when
// using the compatability profile, but if we use the core profile then we'll
// have to create them manually ourselves

using namespace gl;

static void GLClearErrors() {
	GLenum error_code;
	do {
		error_code = glGetError();
	} while (error_code != GL_NO_ERROR);
}

static void GLPrintError(GLenum error_code) {
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

static void GLCheckErrors() {
	GLenum error_code;
	do {
		error_code = glGetError();
		GLPrintError(error_code);
		// std::cerr << "GLError! Code: " << erorr_code << std::endl;
	} while (error_code != GL_NO_ERROR);
}

static bool GLLogCall(const char *function, const char *file, int line) {
	GLenum error_code = glGetError();

	if (error_code == gl::GLenum::GL_NO_ERROR)
		return true;

	GLPrintError(error_code);
	std::cerr << function << " " << file << ":" << line << std::endl;

	return false;
}

struct ShaderProgramSource {
	std::string vertexSource;
	std::string fragmentSource;
};

static ShaderProgramSource ParseShader(const std::string &filePath) {
	std::ifstream stream(filePath);

	enum class ShaderType { NONE = -1, VERTEX, FRAGMENT };

	std::string line;
	std::array<std::stringstream, 2> ss;
	ShaderType shaderType = ShaderType::NONE;

	while (std::getline(stream, line)) {
		if (line.find("#shader") != std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				// set mode to vertex
				shaderType = ShaderType::VERTEX;
			} else if (line.find("fragment") != std::string::npos) {
				// set mode to fragment
				shaderType = ShaderType::FRAGMENT;
			}
		} else {
			ss.at(int(shaderType)) << line << std::endl;
		}
	}

	return {ss[0].str(), ss[1].str()};
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
}

static GLuint CompileShader(gl::GLenum type, const std::string &source) {
	GLuint id = GLCallV(glCreateShader(type));
	const char *const src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	// Retrieve the result of the compilation
	GLboolean result = 0;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE) {
		// Shader didn't compile successfully
		// Query the error message's length
		int length = -1;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		// NOLINTNEXTLINE
		char *message = (char *)alloca((unsigned int)length * sizeof(char));
		// Returns the information log for a shader object
		GLCall(glGetShaderInfoLog(id, length, nullptr, message));
		std::cerr << "Failed to compile "
				  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
				  << " shader!" << std::endl
				  << message << std::endl;
		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

// Compile the two shaders and link them together into a single program and
// return its id
static GLuint CreateProgram(const std::string &vertexShaderSrc,
							const std::string &fragmentShaderSrc) {
	GLuint program = GLCallV(glCreateProgram());

	// Create the two shader objects
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

	// TODO: Error handling

	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

	return program;
}

int main() {
	GLFWwindow *window = nullptr;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	glbinding::initialize(glfwGetProcAddress);

	std::cout << glGetString(GL_VERSION) << std::endl;

	GLCall(glViewport(0, 0, 640, 480));
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Create a vertex buffer in the ram
	std::array<GLfloat, 12> vertex_pos{
		// clang-format off
		-0.5f, 	-0.5f,	// 0
		0.5f, 	-0.5f,	// 1
		0.5f, 	0.5f,	// 2
		-0.5f, 	0.5f,	// 3
		// clang-format on
	};

	// CPU index buffer
	std::array<GLuint, 6> vertex_indices{
		// clang-format off
		0, 1, 2,
		2, 3, 0
		// clang-format on
	};

	GLuint vao = 0;
	GLCall(glGenVertexArrays(1, &vao));
	GLCall(glBindVertexArray(vao));

	GLuint buffer = 0;
	// This function generates a buffer and stores
	// its id in the second argument
	GLCall(glGenBuffers(1, &buffer));
	// Select the buffer
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
	// Set the buffer data in vram
	GLCall(glBufferData(GL_ARRAY_BUFFER,
						vertex_pos.size() *
							sizeof(decltype(vertex_pos)::value_type),
						vertex_pos.data(), GL_STATIC_DRAW));

	// Set the vertex attributes for the bound buffer
	GLCall(glEnableVertexAttribArray(0));
	// This is where the vertex buffer is bound to the currently bound vao
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
								 (const void *)0)); // NOLINT

	// Index-buffer object
	gl::GLuint ibo = 0;
	GLCall(glGenBuffers(1, &ibo));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
						vertex_indices.size() *
							sizeof(decltype(vertex_indices)::value_type),
						vertex_indices.data(), GL_STATIC_DRAW));

	auto [vertexShaderSrc, fragmentShaderSrc] =
		ParseShader("res/shaders/Basic.shader");
	GLuint program = CreateProgram(vertexShaderSrc, fragmentShaderSrc);
	GLCall(glUseProgram(program));

	// We need to have a shader that is bound to set a uniform

	// Once the shader is created every uniform gets assigned an id
	// so we need to retrieve this id
	int colorUniformLocation =
		GLCallV(glGetUniformLocation(program, "u_Color"));
	ASSERT(colorUniformLocation != -1);

	// Unbind all objects
	GLCall(glBindVertexArray(0));
	GLCall(glUseProgram(0));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

	float r = 0.0f;
	float increment = 0.01f;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		GLCall(glUseProgram(program));
		GLCall(glUniform4f(colorUniformLocation, r, 0.3f, 0.8f, 1.0f));

		GLCall(glBindVertexArray(vao));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

		// This is for the case when using index buffers
		GLCall(glDrawElements(
			GL_TRIANGLES, vertex_indices.size(), GLenum::GL_UNSIGNED_INT,
			nullptr)); // Last arguments is null since we already bound the ibo

		if (r > 1.0f || r < 0.0f)
			increment = -increment;

		r += increment;

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	// TODO: Need to cleanup shaders as well
	GLCall(glDeleteProgram(program));
	glfwTerminate();
	return 0;
}