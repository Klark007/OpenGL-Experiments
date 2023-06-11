#define GLFW_INCLUDE_NONE // disables glfw to include glad on it's own. order is now not important
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Program.h"

#include <iostream>

void glfw_error_callback(int error, const char* description);

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main()
{
	// init glad and glfw
	if (!glfwInit()) {
		std::cerr << "GLFW Initalization failed" << std::endl;
		return -1;
	}
	glfwSetErrorCallback(glfw_error_callback);

	// require minimum openGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Experiments", NULL, NULL);

	if (!window) {
		std::cerr << "GLFW Window creation failed" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// needs an active glfw context
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

	glfwSwapInterval(1); // default swap interval is 0 which can lead to screen tearing


	float vertex_data[] = {
		0.25f, 0.25f, 0.0f, // bottom left
		0.75f, 0.25f, 0.0f, // bottom right
		0.75f, 0.75f, 0.0f, // top right
		0.25f, 0.75f, 0.0f, // top left
	};
		
	unsigned int indices[] = {
		0,1,2,
		0,2,3
	};

	// vertex array object
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// buffer vertex data
	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo); // data buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);

	// configure attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);

	// buffer index
	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	/*
	glBindVertexArray(0); // needs to happen before unbinding of vbo and ebo
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	*/

	// shaders
	
	Shader vertex_shader{ GL_VERTEX_SHADER };
	vertex_shader.add_source_from_file("./shaders/base.vs");
	vertex_shader.compile();
	vertex_shader.print_compile_error();

	Shader fragment_shader{ GL_FRAGMENT_SHADER };
	fragment_shader.add_source_from_file("./shaders/uniform_color.fs");
	fragment_shader.compile();
	fragment_shader.print_compile_error();

	Program program = {};
	program.attach_shader(vertex_shader);
	program.attach_shader(fragment_shader);
	program.link_program();
	program.print_link_error();

	program.use();
	program.set1f("offset", -0.5);
	
	std::cout << "Finished preprocessing" << glGetError() << " " << GL_NO_ERROR << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		// input

		
		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// drawArray is for without indices, drawElements for indexed drawing
		program.use();
		glBindVertexArray(vao);

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

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