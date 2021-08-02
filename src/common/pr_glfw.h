#include "GLFW/glfw3.h"
#include <cassert>
#include <string>

namespace GLFWObjects {
class Window {
	friend class GLFW;

  public:
	Window(int width, int height, std::string_view title);
	bool isValid();
	bool shouldClose();
	void swapBuffers();
	void setFramebufferSizeCallback(GLFWframebuffersizefun callback);

  private:
	GLFWwindow *window;
};

class GLFW {
  private:
	GLFW() = default;

  public:
	static GLFW &getInstance();

	enum class WindowHint {
		CONTEXT_VERSION_MAJOR,
		CONTEXT_VERSION_MINOR,
		OPENGL_PROFILE
	};

	enum class OpenGL_Profile { OPENGL_CORE_PROFILE, OPENGL_COMPAT_PROFILE };

	int init();
	template <typename T> void setWindowHint(WindowHint hint, T value) {
		if constexpr (std::is_same_v<T, OpenGL_Profile>) {
			assert(hint == WindowHint::OPENGL_PROFILE);
			if (value == OpenGL_Profile::OPENGL_CORE_PROFILE)
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			else
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
		} else if constexpr (std::is_same_v<T, int>) {
			assert(hint == WindowHint::CONTEXT_VERSION_MAJOR ||
				   hint == WindowHint::CONTEXT_VERSION_MINOR);
			if (hint == WindowHint::CONTEXT_VERSION_MAJOR) {
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, value);
			} else {
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, value);
			}
		} else {
			static_assert(std::is_same_v<T, void>, "Bad call");
		}
	}
	void makeContextCurrent(const Window &window);

	GLFW(const GLFW &other) = delete;
	GLFW(const GLFW &&other) = delete;
	GLFW &operator=(const GLFW &other) = delete;
	GLFW &operator=(const GLFW &&other) = delete;

	~GLFW();
};

} // namespace GLFWObjects