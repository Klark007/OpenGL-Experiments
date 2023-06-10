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
	const char* vertex_shader_code = "#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
		"}\0";

	unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL); // reads in 1 null terminated char*
	glCompileShader(vertex_shader);
	// check success
	int vertex_compiled_status;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_compiled_status);
	if (vertex_compiled_status != GL_TRUE) {
		char vertex_log[512];
		glGetShaderInfoLog(vertex_shader, 512, NULL, vertex_log);
		std::cerr << "Vertex shader compilation failed: " << vertex_log << std::endl;
	}


	const char* fragment_shader_code = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.3f, 1.0f);\n"
		"}\0";
	
	unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
	glCompileShader(fragment_shader);

	int fragment_compiled_status;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fragment_compiled_status);
	if (fragment_compiled_status != GL_TRUE) {
		char fragment_log[512];
		glGetShaderInfoLog(fragment_shader, 512, NULL, fragment_log);
		std::cerr << "Fragment shader compilation failed: " << fragment_log << std::endl;
	}

	unsigned int program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	int program_link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &program_link_status);
	if (program_link_status != GL_TRUE) {
		char program_log[512];
		glGetProgramInfoLog(program, 512, NULL, program_log);
		std::cerr << "Program linking failed: " << program_log << std::endl;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	std::cout << "Finished preprocessing" << glGetError() << " " << GL_NO_ERROR << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		// input

		
		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// drawArray is for without indices, drawElements for indexed drawing
		glUseProgram(program);
		glBindVertexArray(vao);
		std::cout << "Load " << glGetError() << " " << GL_INVALID_ENUM << std::endl;

		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		std::cout << "Draw " << glGetError() << " " << GL_INVALID_ENUM << std::endl;

		// buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(program);

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