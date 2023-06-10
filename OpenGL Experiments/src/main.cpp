# include <iostream>

#define GLFW_INCLUDE_NONE // disables glfw to include glad on it's own. order is now not important
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void glfw_error_callback(int error, const char* description);

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main()
{
	// init glad and glfw
	if (!glfwInit()) {
		std::cout << "GLFW Initalization failed" << std::endl;
		return -1;
	}
	glfwSetErrorCallback(glfw_error_callback);

	// require minimum openGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Experiments", NULL, NULL);

	if (!window) {
		std::cout << "GLFW Window creation failed" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// needs an active glfw context
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

	glfwSwapInterval(1); // default swap interval is 0 which can lead to screen tearing


	while (!glfwWindowShouldClose(window))
	{
		// input

		
		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// destroy window and clean up resources
	glfwTerminate();
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}