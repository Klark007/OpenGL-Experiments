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
#include <direct.h> // windows get cwd

void glfw_error_callback(int error, const char* description);
void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void handle_input(GLFWwindow* window);

unsigned int loadTexture(const char* path);

int screen_x = 800;
int screen_y = 600;

Camera camera = { glm::vec3(0.0, 2.0, 4.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0) };

int main()
{
	char* cwd_buffer;
	if (!(cwd_buffer = _getcwd(NULL, 0))) {
		std::cerr << "CWD Buffer allocation failed" << std::endl;
		return -1;
	}


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

	// disable cursor so can use raw mouse movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (!glfwRawMouseMotionSupported()) {
		std::cerr << "Raw mouse motion not supported" << std::endl;
		return -1; // could fall back on just cursor position
	}

	glfwSetCursorPosCallback(window, glfm_mouse_move_callback);

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
	unsigned int texture_diffuse = loadTexture("textures/container_diffuse.png");
	unsigned int texture_specular = loadTexture("textures/container_specular.png");

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
	vertex_shader.add_source_from_file("shaders/phong.vs");
	std::cout << "Vertex shader compilation" << std::endl;
	vertex_shader.compile();
	vertex_shader.print_compile_error();

	Shader fragment_shader{ GL_FRAGMENT_SHADER };
	std::cout << "Fragment shader compilation" << std::endl;
	fragment_shader.add_source_from_file("shaders/phong.fs");
	fragment_shader.compile();
	fragment_shader.print_compile_error();

	Program program = {};
	program.attach_shader(vertex_shader);
	program.attach_shader(fragment_shader);
	program.link_program();
	program.print_link_error();

	glm::mat4 model = glm::mat4(1.0);
	//model = glm::rotate(model, glm::radians(45.0f), glm::normalize(glm::vec3(0.0, 0.0, 1.0)));

	glm::mat4 view = camera.generate_view_mat();

	glm::mat4 projection;
	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.01f, 100.0f);

	program.use();
	program.set_mat4f("model", model);
	program.set_mat4f("view", view);
	program.set_mat4f("projection", projection);

	// potential driver bug with texture unit 0 being used if active
	program.set1i("mat.diffuse_texture", 0);
	program.set1i("mat.specular_texture", 1);

	program.set1f("mat.shininess", 64.0);

	program.set_vec3f("light.diffuse", 0.65f, 0.65f, 0.65f);
	program.set_vec3f("light.specular", 1.0, 1.0, 1.0);
	program.set_vec3f("light.pos", 1.2, 1.0, 2.0);

	std::cout << "Finished preprocessing" << glGetError() << " " << GL_NO_ERROR << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		// input
		handle_input(window);
		
		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_diffuse);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_specular);

		program.set_mat4f("model", model);

		view = camera.generate_view_mat();
		program.set_mat4f("view", view);

		glm::vec3 camera_pos = camera.get_pos();
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

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	screen_x = width;
	screen_y = height;
}

void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	const double strength = 0.001;
	static double xlast = screen_x / 2;
	static double ylast = screen_y / 2;
	
	double dx = xpos - xlast;
	double dy = ypos - ylast;

	glm::vec3 dir = camera.get_dir();

	// not efficient, could compute once and update
	double yaw   = atan2(dir.z, dir.x);
	double pitch = asin(dir.y);

	yaw   += dx * strength;
	pitch += -dy * strength;

	dir.x = (float) (cos(yaw) * cos(pitch));
	dir.y = (float) sin(pitch);
	dir.z = (float) (sin(yaw) * cos(pitch));

	camera.set_dir(dir);

	xlast = xpos;
	ylast = ypos;
}

void handle_input(GLFWwindow* window)
{
	const float strength = 0.1f;
	
	float dx = 0.0f;
	float dy = 0.0f;

	if (glfwGetKey(window, GLFW_KEY_W))
		dx += 1;
	if (glfwGetKey(window, GLFW_KEY_S))
		dx -= 1;
	if (glfwGetKey(window, GLFW_KEY_D))
		dy += 1;
	if (glfwGetKey(window, GLFW_KEY_A))
		dy -= 1;

	glm::vec3 pos = camera.get_pos();
	glm::vec3 dir = camera.get_dir();

	pos += dx * strength * dir;
	glm::vec3 side = glm::cross(dir, camera.get_up()); // can assume both normalized
	pos += dy * strength * side;

	camera.set_pos(pos);
}
