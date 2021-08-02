// clang-format off
#include <glbinding/gl/gl.h>
#include <glbinding/glbinding.h>
#define GLFW_INCLUDE_NONE
#include "pr_glfw.h"
// clang-format on
#include <array>
#include <fstream>
#include <iostream>
#include <sstream>

#include "indexbuffer.h"
#include "renderer.h"
#include "vertexbuffer.h"

// Documentation website: docs.gl

// Summary:

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
	/* Initialize glfw */
	GLFWObjects::GLFW &glfw = GLFWObjects::GLFW::getInstance();
	if (!glfw.init())
		return -1;

	glfw.setWindowHint(GLFWObjects::GLFW::WindowHint::CONTEXT_VERSION_MAJOR, 3);
	glfw.setWindowHint(GLFWObjects::GLFW::WindowHint::CONTEXT_VERSION_MINOR, 3);
	// glfw.setWindowHint(GLFWObjects::GLFW::WindowHint::OPENGL_PROFILE,
	// 				   GLFWObjects::GLFW::OpenGL_Profile::OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	GLFWObjects::Window window(640, 480, "Hello World");
	if (!window.isValid()) {
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfw.makeContextCurrent(window);

	glfwSwapInterval(1);

	glbinding::initialize(glfwGetProcAddress);

	std::cout << glGetString(GL_VERSION) << std::endl;

	GLCall(glViewport(0, 0, 640, 480));
	window.setFramebufferSizeCallback(framebuffer_size_callback);

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

	VertexBuffer vb(vertex_pos.data(),
					vertex_pos.size() *
						sizeof(decltype(vertex_pos)::value_type));

	// Set the vertex attributes for the bound buffer
	GLCall(glEnableVertexAttribArray(0));
	// This is where the vertex buffer is bound to the currently bound vao
	GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
								 (const void *)0)); // NOLINT

	IndexBuffer ib(vertex_indices.data(), vertex_indices.size());

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
	while (!window.shouldClose()) {
		/* Render here */
		GLCall(glClear(GL_COLOR_BUFFER_BIT));

		GLCall(glUseProgram(program));
		GLCall(glUniform4f(colorUniformLocation, r, 0.3f, 0.8f, 1.0f));

		GLCall(glBindVertexArray(vao));
		ib.bind();

		// This is for the case when using index buffers
		GLCall(glDrawElements(
			GL_TRIANGLES, vertex_indices.size(), GLenum::GL_UNSIGNED_INT,
			nullptr)); // Last arguments is null since we already bound the ibo

		if (r > 1.0f || r < 0.0f)
			increment = -increment;

		r += increment;

		/* Swap front and back buffers */
		window.swapBuffers();

		/* Poll for and process events */
		glfwPollEvents();
	}

	// TODO: Need to cleanup shaders as well
	GLCall(glDeleteProgram(program));
	return 0;
}