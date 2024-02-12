#define GLFW_INCLUDE_NONE // disables glfw to include glad on it's own. order is now not important
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "Model.h"
#include "Material.h"
#include "Texture.h"

#include "Shader.h"
#include "program.h"
#include "Camera.h"

#include "sampling/fdps.h"
#define NR_SHADOW_SAMPLES 16
#define NR_OCCLUDER_SAMPLES 16

#include "noise/PerlinWorley3D.h"
#include "noise/PerlinFBM.h"

#include <iostream>
#include <memory>

#include <chrono>
typedef std::chrono::steady_clock::time_point time_point;

inline long long duration(time_point a, time_point b) {
	return std::chrono::duration_cast<std::chrono::seconds>(b - a).count();
}

void glfw_error_callback(int error, const char* description);
void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void handle_input(GLFWwindow* window);

int screen_x = 900;
int screen_y = 600;
bool resize = false;

Camera camera = { glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0) };

bool can_move = true;

int worley_channel = -1;
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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POLYGON_OFFSET_FILL);

	bool recompile_shaders = false;

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
	constexpr float far_plane = 500.0f;
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
	Texture frame_color_t = Texture(
		std::string("frame"),
		screen_x,
		screen_y,
		(unsigned char*)0,
		GL_RGB,
		GL_RGB,
		Texture_Filter::linear,
		Texture_Wrap::clamp
	);

	Texture frame_depth_t = Texture(
		std::string("depth"),
		screen_x,
		screen_y,
		(void*)0,
		GL_UNSIGNED_INT_24_8,
		GL_DEPTH24_STENCIL8,
		GL_DEPTH_STENCIL,
		Texture_Filter::nearest,
		Texture_Wrap::clamp
	);
	
	glBindFramebuffer(GL_FRAMEBUFFER, main_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame_color_t.get_id(), 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, frame_depth_t.get_id(), 0);


	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Main frame buffer creation failed with status: " << status << std::endl;
		return -1;
	}

	
	unsigned int noise_res_x = 128;
	unsigned int noise_res_y = 128;
	unsigned int noise_res_z = 128;
	PerlinWorley3D<float> w(noise_res_x, noise_res_y, noise_res_z, 2, 2, 2, { {8,8,8}, {10,10,10}, {16,16,16}, {20,20,20} });
	Texture lf_texture = Texture(
		std::string("worley_n"),
		noise_res_x,
		noise_res_y,
		noise_res_z,
		w.get_data(),
		GL_RGBA,
		GL_RGBA,
		Texture_Filter::linear,
		Texture_Wrap::repeat
	);

	unsigned int high_freq_res = 32;
	PerlinWorley3D<float> w2(high_freq_res, high_freq_res, high_freq_res, 2, 2, 2, { {1,1,1}, {12,12,12}, {16,16,16}, {6,6,6} });
	w2.set_channel(0,w2.get_channel(2));

	Texture hf_texture = Texture(
		std::string("high_freq_n"),
		high_freq_res,
		high_freq_res,
		high_freq_res,
		w2.get_data(),
		GL_RGBA,
		GL_RGBA,
		Texture_Filter::linear,
		Texture_Wrap::repeat
	);

	unsigned int weather_res_x = 256;
	unsigned int weather_res_y = 256;
	PerlinFBM<float> high_coverage_map(weather_res_x, weather_res_y, 2, 2, 5);
	Texture weather_texture = Texture(
		std::string("weather_map"),
		weather_res_x,
		weather_res_y,
		high_coverage_map.get_data(),
		GL_RGB,
		GL_RED,
		Texture_Filter::linear,
		Texture_Wrap::repeat
	);

	// light dir
	glm::vec3 sun_dir = glm::vec3(0,1,0);
	glm::vec3 cloud_color = glm::vec3(1);
	float cloud_light_strength = 3.3;
	glm::vec3 cloud_ambient = glm::vec3(0.5);

	float cloud_global_coverage = 1.0;
	float cloud_global_density = 25.0;

	float cloud_lf_scale = 0.059;
	float cloud_hf_scale = 0.082;
	float weather_scale = 0.014;

	float phase_eccentricity_g = 0.2;

	int cloud_use_phase_function = 1;

	int raymarch_steps = 64;
	float cloud_jitter = 0.9; // balance between noise and aliasing in form of rings

	int adaptive_stepsize = 0;
	float coarse_stepsize_scale = 4.0;
	int nr_misses_coarse_switch = 5;

	
	int do_incr_sz_far = 1;
	int inv_incr_sz_far_away = 500;

	int do_early_termination = 1;
	int inv_early_termination = 100;

	float cloud_height_min = 20.0;
	float cloud_height_max = 25.0;

	std::vector<std::shared_ptr<Shader>> post_shaders;
	post_shaders.push_back(std::make_shared<Shader>(GL_VERTEX_SHADER, "shaders/volume.vs"));
	post_shaders.push_back(std::make_shared<Shader>(GL_FRAGMENT_SHADER, "shaders/volume.fs"));
	Program post_program{ post_shaders };

	post_program.use();
	frame_color_t.set_texture_unit(post_program, 2);
	frame_depth_t.set_texture_unit(post_program, 3);

	lf_texture.set_texture_unit(post_program, 4);
	hf_texture.set_texture_unit(post_program, 5);
	weather_texture.set_texture_unit(post_program, 6);
		
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
		
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			recompile_shaders = true;
		}
		if (recompile_shaders && glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
			std::cout << "Recompile" << std::endl;

			/*
			for (std::shared_ptr<Shader>& shader : post_shaders) {
				shader->recompile();
			}
			*/
			post_program.recompile();

			recompile_shaders = false;
		}

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
		
		if (resize) {
			std::cout << "RESIZE:"  << screen_x << "," << screen_y << std::endl;
			frame_color_t.resize(screen_x, screen_y);
			frame_depth_t.resize(screen_x, screen_y);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, main_buffer);
		glViewport(0, 0, screen_x, screen_y);

		// rendering
		glClearColor(0.3, 0.65, 1.0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		program.use();
		program.set1f("time", glfwGetTime());

		glPolygonOffset(0.0f, 0.0f);

		if (resize) {
			projection = glm::perspective(fov_y, ((float)screen_x) / screen_y, near_plane, far_plane);
			program.set_mat4f("projection", projection);
		}

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

		frame_color_t.bind();
		frame_depth_t.bind();

		// interactive:
		{
			post_program.set1i("worley_channel", worley_channel);
			post_program.set_vec3f("worley_offset", worley_offset);
			
			post_program.set_vec3f("light_dir", sun_dir);
			post_program.set_vec3f("light_color", cloud_color);
			post_program.set_vec3f("light_albedo", cloud_ambient);
			post_program.set1f("light_strength", cloud_light_strength);
			
			post_program.set1f("global_coverage", cloud_global_coverage);
			post_program.set1f("global_density", cloud_global_density);

			post_program.set1f("low_freq_scale", cloud_lf_scale);
			post_program.set1f("high_freq_scale", cloud_hf_scale);
			post_program.set1f("weather_scale", weather_scale);

			post_program.set1i("use_phase_function", cloud_use_phase_function);
			post_program.set1f("phase_eccentricity", phase_eccentricity_g);

			post_program.set1i("nr_steps", raymarch_steps);
			post_program.set1f("jitter_str", cloud_jitter);

			post_program.set1i("adaptive_stepsize", adaptive_stepsize);
			post_program.set1f("coarse_multiplier", coarse_stepsize_scale);
			post_program.set1i("nr_misses_switch", nr_misses_coarse_switch);

			post_program.set1i("do_distance_step_size", do_incr_sz_far);
			post_program.set1i("do_early_termination", do_early_termination);
			post_program.set1f("dist_incr_step_size", 1.0 / inv_incr_sz_far_away);
			post_program.set1f("early_termination", 1.0 / inv_early_termination);

			post_program.set_vec2f("cloud_bounding_box.min_max[1]", cloud_height_min, cloud_height_max);
			post_program.set_vec2f("cloud_min_max_height", cloud_height_min, cloud_height_max);
		}

		lf_texture.bind();
		hf_texture.bind();
		weather_texture.bind();
		
		ground_plane.draw(post_program);

		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			// Group into subtabs
			ImGui::Begin("Edit settings (Clouds)");

			if (ImGui::CollapsingHeader("Lighting Settings"))
			{
				ImGui::SliderFloat3("Sun Direction", (float*)&sun_dir, -1, 1);
				ImGui::ColorEdit3("Cloud color", (float*)&cloud_color);
				ImGui::SliderFloat("Light strength", &cloud_light_strength, 0.0f, 10.0f);
				ImGui::ColorEdit3("Cloud ambient", (float*)&cloud_ambient);
				
				ImGui::Checkbox("Phase function", (bool*)&cloud_use_phase_function);
				ImGui::SliderFloat("Phase eccentrity", &phase_eccentricity_g, -1.0f, 1.0f);
			}

			if (ImGui::CollapsingHeader("Raymarching Settings"))
			{
				ImGui::SliderInt("Number of raymarch steps", &raymarch_steps, 4, 128);
				ImGui::SliderFloat("Cloud Jitter", &cloud_jitter, 0.0f, 1.0f);
			}

			if (ImGui::CollapsingHeader("Density/Coverage Settings"))
			{
				ImGui::SliderFloat("Global coverage", &cloud_global_coverage, 0.0f, 1.0f);
				ImGui::SliderFloat("Global density", &cloud_global_density, 0.0f, 50.0f);
				
				ImGui::SliderFloat("Low freq scale", &cloud_lf_scale, 0.0f, 1.0f);
				ImGui::SliderFloat("High freq scale", &cloud_hf_scale, 0.0f, 1.0f);
				ImGui::SliderFloat("Weather scale", &weather_scale, 0.0f, 1.0f);

				ImGui::SliderFloat("Cloud Min Height", &cloud_height_min, 0.0f, 100.0f);
				ImGui::SliderFloat("Cloud Max Height", &cloud_height_max, 0.0f, 100.0f);

			}
			
			if (ImGui::CollapsingHeader("Optimization Settings"))
			{
				ImGui::Checkbox("Adaptive Stepsize", (bool*)&adaptive_stepsize);
				ImGui::SliderFloat("Coarse stepsize multiplier", &coarse_stepsize_scale, 1.0f, 8.0f);
				ImGui::SliderInt("Misses until coarse switch", &nr_misses_coarse_switch, 2, 8);
				
				ImGui::Checkbox("Distance based increase of stepsize", (bool*)&do_incr_sz_far);
				ImGui::SliderInt("Inv Distance increase stepsize", &inv_incr_sz_far_away, 300, 800);

				ImGui::Checkbox("Early termination", (bool*)&do_early_termination);
				ImGui::SliderInt("Inv Early Termination", &inv_early_termination, 50, 1000);
			}

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		resize = false;

		// buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();


	// destroy window and clean up resources
	glfwDestroyWindow(window);
	glfwTerminate();
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	resize = true;
	screen_x = width;
	screen_y = height;
}

void glfm_mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	const double strength = 0.001;
	static double xlast = screen_x / 2;
	static double ylast = screen_y / 2;

	if (!can_move) {
		xlast = xpos;
		ylast = ypos;
		return;
	}

	
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
	// to get comparison pictures
	if (glfwGetKey(window, GLFW_KEY_X)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		can_move = false;
	}
	if (glfwGetKey(window, GLFW_KEY_Y)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
		worley_channel = -1;
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
	if (glfwGetKey(window, GLFW_KEY_4)) {
		worley_channel = 3;
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
