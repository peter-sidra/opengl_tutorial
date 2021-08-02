#include "pr_glfw.h"
#include <type_traits>

namespace GLFWObjects {
Window::Window(int width, int height, std::string_view title) {
	window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
}

bool Window::isValid() {
	return window;
}

bool Window::shouldClose() {
	return glfwWindowShouldClose(window);
}

void Window::swapBuffers() {
	glfwSwapBuffers(window);
}

void Window::setFramebufferSizeCallback(GLFWframebuffersizefun callback) {
	glfwSetFramebufferSizeCallback(window, callback);
}

GLFW &GLFW::getInstance() {
	static GLFW instance;
	return instance;
}

int GLFW::init() {
	return glfwInit();
}

void GLFW::makeContextCurrent(const Window &window) {
	glfwMakeContextCurrent(window.window);
}

GLFW::~GLFW() {
	glfwTerminate();
}
} // namespace GLFWObjects