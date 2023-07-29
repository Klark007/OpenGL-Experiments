#define GLFW_INCLUDE_NONE // disables glfw to include glad on it's own. order is now not important
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.h"
#include "Program.h"
#include "Camera.h"

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


	glEnable(GL_DEPTH_TEST);

	float vertex_data[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};
		
	/*
	unsigned int indices[] = {
		0,1,2,
		0,2,3
	};
	*/

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// textures
	const std::string path = "textures/logo.png";
	int width, height, channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

	unsigned int texture;
	glGenTextures(1, &texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	if (data == NULL) {
		std::cerr << "Image not found at: " << path << std::endl;
	}
	else {
		// should be GL_RGB32F, GL_BGR
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D); // is required unless manually defined for needed levels
	}

	stbi_image_free(data);

	/*
	// buffer index
	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); // index buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	*/

	/*
	glBindVertexArray(0); // needs to happen before unbinding of vbo and ebo
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	*/

	// shaders
	
	Shader vertex_shader{ GL_VERTEX_SHADER };
	vertex_shader.add_source_from_file("./shaders/texture.vs");
	vertex_shader.compile();
	vertex_shader.print_compile_error();

	Shader fragment_shader{ GL_FRAGMENT_SHADER };
	fragment_shader.add_source_from_file("./shaders/texture.fs");
	fragment_shader.compile();
	fragment_shader.print_compile_error();

	Program program = {};
	program.attach_shader(vertex_shader);
	program.attach_shader(fragment_shader);
	program.link_program();
	program.print_link_error();

	glm::mat4 model = glm::mat4(1.0);
	//model = glm::rotate(model, glm::radians(45.0f), glm::normalize(glm::vec3(0.0, 0.0, 1.0)));

	Camera camera = {glm::vec3(0.0, 2.0, 4.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)};
	glm::mat4 view = camera.generate_view_mat();

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.01f, 100.0f);

	program.use();
	program.set_mat4f("model", model);
	program.set_mat4f("view", view);
	program.set_mat4f("projection", projection);

	program.set_vec3f("mat.diffuse", 1.0, 0.2, 0.2);
	program.set_vec3f("mat.specular", 1.0, 0.8, 0.8);
	program.set1f("mat.shininess", 32.0);

	program.set_vec3f("light.diffuse", 1.0, 1.0, 1.0);
	program.set_vec3f("light.specular", 1.0, 1.0, 1.0);
	program.set_vec3f("light.pos", 1.2, 1.0, 2.0);

	glUniform1i(glGetUniformLocation(program.get_id(), "color_texture"), 0); // set it manually

	std::cout << "Finished preprocessing" << glGetError() << " " << GL_NO_ERROR << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		// input

		
		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		program.set_mat4f("model", model);

		glm::vec3 camera_pos = glm::vec3(5 * std::cos(glfwGetTime()), -2.0, 5 * std::sin(glfwGetTime()));
		camera.set_pos(camera_pos);
		view = camera.generate_view_mat();
		program.set_mat4f("view", view);

		program.set_vec3f("camera_pos", camera_pos);

		// drawArray is for without indices, drawElements for indexed drawing
		
		glBindVertexArray(vao);
		//glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertex_data) / sizeof(float));

		glm::mat4 model2 = glm::mat4(1.0);
		model2 = glm::translate(model2, glm::vec3(1.2, 1.0, 2.0));
		program.set_mat4f("model", model2);

		glDrawArrays(GL_TRIANGLES, 0, sizeof(vertex_data) / sizeof(float));


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