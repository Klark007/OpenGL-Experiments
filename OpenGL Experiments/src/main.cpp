#define GLFW_INCLUDE_NONE // disables glfw to include glad on it's own. order is now not important
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "Model.h"
#include "Material.h"

#include "Shader.h"
#include "program.h"
#include "Camera.h"

#include "sampling/fdps.h"
#define NR_SHADOW_SAMPLES 16
#define NR_OCCLUDER_SAMPLES 16

#include "noise/Worley-Noise/Worley Noise/Worley3D.h"
#include "noise/Perlin.h"

#include <iostream>
#include <memory>

void glfw_error_callback(int error, const char* description);
void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void handle_input(GLFWwindow* window);

int screen_x = 800;
int screen_y = 600;

Camera camera = { glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0) };

bool can_move = true;

int worley_channel = 0;
glm::vec3 worley_offset = glm::vec3(0.0, 0.0, 0.0);

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

	GLFWwindow* window = glfwCreateWindow(screen_x, screen_y, "OpenGL Experiments", NULL, NULL);

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
	glEnable(GL_POLYGON_OFFSET_FILL);

	Model backpack = { "objects/backpack/backpack.obj" };

	std::vector<Vertex> vertices = {
		{glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,1.0,0.0), glm::vec2(0.0,0.0)},
		{glm::vec3(0.0,0.0,1.0), glm::vec3(0.0,1.0,0.0), glm::vec2(0.0,1.0)},
		{glm::vec3(1.0,0.0,0.0), glm::vec3(0.0,1.0,0.0), glm::vec2(1.0,0.0)},
		{glm::vec3(1.0,0.0,1.0), glm::vec3(0.0,1.0,0.0), glm::vec2(1.0,1.0)}
	};
	std::vector<unsigned int> indices = { 0,1,2,1,3,2 };
	std::shared_ptr<Material> material = std::make_shared<Material>(std::vector<std::pair<std::string, std::string>> { {"diffuse", "white.png"}, {"specular", "container_specular.png"}}, "textures");
	Mesh ground_plane {vertices, indices, material};
	glm::mat4 ground_model = glm::mat4(1.0);
	ground_model = glm::translate(ground_model, glm::vec3(-5, -1.8, -5));
	ground_model = glm::scale(ground_model, glm::vec3(10, 1, 10));

	// shaders
	std::vector<std::shared_ptr<Shader>> phong_shaders;
	phong_shaders.push_back(std::make_shared<Shader>(GL_VERTEX_SHADER, "shaders/phong.vs"));
	phong_shaders.push_back(std::make_shared<Shader>(GL_FRAGMENT_SHADER, "shaders/phong.fs"));
	Program program{ phong_shaders };


	glm::mat4 model = glm::mat4(1.0);
	glm::mat4 view = camera.generate_view_mat();
	glm::mat4 projection;

	constexpr float fov_y = glm::radians(45.0f);
	constexpr float near_plane = 0.1f;
	constexpr float far_plane = 100.0f;
	projection = glm::perspective(fov_y, ((float) screen_x) / screen_y, near_plane, far_plane);

	program.use();
	program.set_mat4f("view", view);
	program.set_mat4f("projection", projection);

	program.set1i("mat.diffuse_0", 0);
	program.set1i("mat.specular_0", 1);

	program.set1f("mat.shininess", 64.0);

	glm::vec3 light_position { 10.0, 8.0, 7.5  };
	glm::mat4 light_model = glm::mat4(1.0);
	light_model = glm::translate(light_model, light_position);
	light_model = glm::scale(light_model, glm::vec3 {0.5f, 0.5f, 0.5f});

	program.set_vec3f("light.diffuse", 0.65f, 0.65f, 0.65f);
	program.set_vec3f("light.specular", 1.0, 1.0, 1.0);
	program.set_vec3f("light.pos", light_position);


	// shadows
	unsigned int shadow_buffer;
	glGenFramebuffers(1, &shadow_buffer);

	unsigned int shadow_texture;
	glGenTextures(1, &shadow_texture);
	glBindTexture(GL_TEXTURE_2D, shadow_texture);

	// different resolution than window
	unsigned int shadow_resolution_x = 1024;
	unsigned int shadow_resolution_y = 1024;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_resolution_x, shadow_resolution_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*) 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // how does low resolution shadow map look if we use linear instead of nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Depth buffer creation failed with status: " << status << std::endl;
		return -1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	std::vector<std::shared_ptr<Shader>> depth_shaders;
	depth_shaders.push_back(std::make_shared<Shader>(GL_VERTEX_SHADER, "shaders/depth.vs"));
	depth_shaders.push_back(std::make_shared<Shader>(GL_FRAGMENT_SHADER,"shaders/depth.fs"));
	Program depth_program { depth_shaders };

	glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.5f, 50.0f);
	glm::mat4 light_view = glm::lookAt(light_position, glm::vec3{0.0f}, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 light_space = light_projection * light_view;

	depth_program.use();
	depth_program.set_mat4f("light_space", light_space);

	program.use();
	program.set_mat4f("light_space", light_space);
	program.set1i("light.shadow_map", 2);


	// render to frame buffer and afterwards render volumetrics ontop
	unsigned int main_buffer;
	glGenFramebuffers(1, &main_buffer);

	// color texture
	unsigned int frame_color;
	glGenTextures(1, &frame_color);
	glBindTexture(GL_TEXTURE_2D, frame_color);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_x, screen_y, 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // how does low resolution shadow map look if we use linear instead of nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	unsigned int frame_ds;
	glGenTextures(1, &frame_ds);
	glBindTexture(GL_TEXTURE_2D, frame_ds);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_x, screen_y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, (void*)0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // how does low resolution shadow map look if we use linear instead of nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, main_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_color, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, frame_ds, 0);


	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Main frame buffer creation failed with status: " << status << std::endl;
		return -1;
	}

	// TODO: Create 2d perlin noise for coverage (should tile)
	// TODO: Create 3d worley-perlin noise for density
	unsigned int noise_res_x = 128;
	unsigned int noise_res_y = 128;
	/*
	unsigned int noise_res_z = 128;
	Worley3D<unsigned char> w(noise_res_x, noise_res_y, noise_res_z, { {16,16,16}, {24,24,24}, {32,32,32} });
	
	unsigned int w_texture;
	glGenTextures(1, &w_texture);
	glBindTexture(GL_TEXTURE_3D, w_texture);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, noise_res_x, noise_res_y, noise_res_z, 0, GL_RGB, GL_UNSIGNED_BYTE, w.get_data());
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // how does low resolution shadow map look if we use linear instead of nearest
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	*/

	unsigned int w_texture;
	glGenTextures(1, &w_texture);

	Perlin<unsigned char> p (128, 128, 10, 10);
	glBindTexture(GL_TEXTURE_2D, w_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, noise_res_x, noise_res_y, 0, GL_RED, GL_UNSIGNED_BYTE, p.get_data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // how does low resolution shadow map look if we use linear instead of nearest
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::vector<std::shared_ptr<Shader>> post_shaders;
	post_shaders.push_back(std::make_shared<Shader>(GL_VERTEX_SHADER, "shaders/volume.vs"));
	post_shaders.push_back(std::make_shared<Shader>(GL_FRAGMENT_SHADER, "shaders/volume.fs"));
	Program post_program{ post_shaders };

	post_program.use();
	post_program.set1i("frame", 2);
	post_program.set1i("depth", 3);
	post_program.set1i("worley_n", 4);
	post_program.set1f("projection.y_fov", fov_y);
	post_program.set1f("projection.d_near", near_plane);
	post_program.set1f("projection.d_far", far_plane);

	std::cout << "Finished preprocessing:" << glGetError() << " " << GL_NO_ERROR << std::endl;

	float slope_scale_bias = 2.5f;
	float constant_bias = 7.0f;

	while (!glfwWindowShouldClose(window))
	{
		// input
		handle_input(window);

		// draw shadow to other textures
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_buffer);
		glViewport(0, 0, shadow_resolution_x, shadow_resolution_y);

		glClear(GL_DEPTH_BUFFER_BIT);
		
		depth_program.use();

		glPolygonOffset(slope_scale_bias, constant_bias);

		depth_program.set_mat4f("model", model);
		backpack.draw(depth_program);
		depth_program.set_mat4f("model", ground_model);
		ground_plane.draw(depth_program);
		

		glBindFramebuffer(GL_FRAMEBUFFER, main_buffer);
		glViewport(0, 0, screen_x, screen_y);

		// rendering
		glClearColor(0.25, 0.5, 0.1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();
		program.set1f("time", glfwGetTime());

		glPolygonOffset(0.0f, 0.0f);

		view = camera.generate_view_mat();
		program.set_mat4f("view", view);

		glm::vec3 camera_pos = camera.get_pos();
		program.set_vec3f("camera_pos", camera_pos);

		// activate shadow map, assumes that materials only use Texture unit 0,1
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadow_texture);

		program.set_mat4f("model", model);
		backpack.draw(program);

		program.set_mat4f("model", light_model);
		backpack.draw(program);

		program.set_mat4f("model", ground_model);
		ground_plane.draw(program);


		// post processing
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screen_x, screen_y);

		// rendering
		glClearColor(0.1, 0.3, 0.1, 1);
		glDisable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		post_program.use();
		post_program.set1f("time", glfwGetTime());
		post_program.set_vec2f("resolution", (float)screen_x, (float)screen_y);
		post_program.set_mat4f("view", view);
		post_program.set_vec3f("w_pos", camera_pos);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, frame_color);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, frame_ds);

		post_program.set1i("worley_channel", worley_channel);
		post_program.set_vec3f("worley_offset", worley_offset);
		glActiveTexture(GL_TEXTURE4);
		//glBindTexture(GL_TEXTURE_3D, w_texture);
		glBindTexture(GL_TEXTURE_2D, w_texture);

		ground_plane.draw(post_program);

		glEnable(GL_DEPTH_TEST);

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
	screen_x = width;
	screen_y = height;
}

void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!can_move)
		return;

	const double strength = 0.001;
	static double xlast = screen_x / 2;
	static double ylast = screen_y / 2;
	
	double dx = xpos - xlast;
	double dy = ypos - ylast;

	glm::vec3 dir = camera.get_dir();

	// not efficient, could comp	ute once and update
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
	// to get comparison pictures
	if (glfwGetKey(window, GLFW_KEY_X)) {
		can_move = false;
	}
	if (glfwGetKey(window, GLFW_KEY_Y)) {
		can_move = true;
	}

	if (!can_move)
		return;

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

	if (glfwGetKey(window, GLFW_KEY_0)) {
		worley_channel = 3;
	}
	if (glfwGetKey(window, GLFW_KEY_1)) {
		worley_channel = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_2)) {
		worley_channel = 1;
	}
	if (glfwGetKey(window, GLFW_KEY_3)) {
		worley_channel = 2;
	}

	if (glfwGetKey(window, GLFW_KEY_J)) {
		worley_offset.x -= 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_L)) {
		worley_offset.x += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_K)) {
		worley_offset.y -= 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_I)) {
		worley_offset.y += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_U)) {
		worley_offset.z -= 0.002;
	}
	if (glfwGetKey(window, GLFW_KEY_O)) {
		worley_offset.z += 0.002;
	}

	glm::vec3 pos = camera.get_pos();
	glm::vec3 dir = camera.get_dir();

	pos += dx * strength * dir;
	glm::vec3 side = glm::cross(dir, camera.get_up()); // can assume both normalized
	pos += dy * strength * side;

	camera.set_pos(pos);
}
