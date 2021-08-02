// clang-format off
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// clang-format on
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

// Documentation website: docs.gl

// Summary: create a buffer that stores the indices of the triangles to draw
// (the index buffer) by storing the indices of the raw vertex data, then draw
// using glDrawElements instead of glDrawArrays

using namespace gl;

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
	GLuint id = glCreateShader(type);
	const char *const src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	// Retrieve the result of the compilation
	GLboolean result = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		// Shader didn't compile successfully
		// Query the error message's length
		int length = -1;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		// NOLINTNEXTLINE
		char *message = (char *)alloca((unsigned int)length * sizeof(char));
		// Returns the information log for a shader object
		glGetShaderInfoLog(id, length, nullptr, message);
		std::cerr << "Failed to compile "
				  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
				  << " shader!" << std::endl
				  << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

// Compile the two shaders and link them together into a single program and
// return its id
static GLuint CreateProgram(const std::string &vertexShaderSrc,
							const std::string &fragmentShaderSrc) {
	GLuint program = glCreateProgram();

	// Create the two shader objects
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vertexShaderSrc);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);

	// TODO: Error handling

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main() {
	GLFWwindow *window = nullptr;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(640, 480, "Hello World", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glbinding::initialize(glfwGetProcAddress);

	std::cout << glGetString(GL_VERSION) << std::endl;

	glViewport(0, 0, 640, 480);
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

	gl::GLuint buffer = 0;
	// This function generates a buffer and stores
	// its id in the second argument
	glGenBuffers(1, &buffer);
	// Select the buffer
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// Set the buffer data in vram
	glBufferData(GL_ARRAY_BUFFER,
				 vertex_pos.size() * sizeof(decltype(vertex_pos)::value_type),
				 vertex_pos.data(), GL_STATIC_DRAW);

	// Set the vertex attributes for the bound buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
						  (const void *)0); // NOLINT

	// Index-buffer object
	gl::GLuint ibo = 0;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 vertex_indices.size() *
					 sizeof(decltype(vertex_indices)::value_type),
				 vertex_indices.data(), GL_STATIC_DRAW);

	auto [vertexShaderSrc, fragmentShaderSrc] =
		ParseShader("res/shaders/Basic.shader");
	std::cout << "Vertex shader: " << vertexShaderSrc << std::endl;
	GLuint program = CreateProgram(vertexShaderSrc, fragmentShaderSrc);
	glUseProgram(program);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window)) {
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT);

		// This is for the case with direct vertex data
		// glDrawArrays(GL_TRIANGLES, 0, vertex_pos.size() / 2);

		// This is for the case when using index buffers
		glDrawElements(
			GL_TRIANGLES, vertex_indices.size(), GLenum::GL_UNSIGNED_INT,
			nullptr); // Last arguments is null since we already bound the ibo

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	// TODO: Need to cleanup shaders as well
	glDeleteProgram(program);
	glfwTerminate();
	return 0;
}