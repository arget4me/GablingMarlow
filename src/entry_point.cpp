#include <Windows.h>

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>


#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>



#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "config.h"
#include "Utils/logfile.h"

#define READFILE_IMPLEMENTATION
#include "Utils/readfile.h"

#define WRITEFILE_IMPLEMENTATION
#include "Utils/writefile.h"

#define VALUE_MODIFIERS_IMPLEMENTATION
#include "Utils/value_modifiers.h"

#include "Renderer/opengl_renderer.h"

#include "Renderer/obj_loader.h"
#include "Renderer/dae_loader.h"

#include "World/world.h"
#include "World/Editor/world_editor.h"

#include "Audio/openal_audio_manager.h"


/*-----------------------------
			Globals
-------------------------------*/
#define GLOBALS_DEFINITIONS
#include "globals.h"

local_scope Camera camera;
local_scope Camera camera_editor;
local_scope Camera camera_object_editor;

local_scope ShaderProgram shader;
local_scope ShaderProgram shader_editor;
local_scope ShaderProgram shader_solid;
local_scope ShaderProgram shader_animation;
local_scope ShaderProgram shader_water;
local_scope ShaderProgram shader_sky;
local_scope ShaderProgram shader_post_processing;
local_scope ShaderProgram shader_copy_output;	

static void error_callback(int error, const char* description)
{
	ERROR_LOG(description);
}

static void setup_shaders()
{
	shader.vertex_source_path = "data/shaders/general_vs.glsl";
	shader.fragment_source_path = "data/shaders/general_fs.glsl";

	shader_editor.vertex_source_path = "data/shaders/general_vs.glsl";
	shader_editor.fragment_source_path = "data/shaders/editor_general_fs.glsl";

	shader_solid.vertex_source_path = "data/shaders/solid_vs.glsl";
	shader_solid.fragment_source_path = "data/shaders/solid_fs.glsl";

	shader_animation.vertex_source_path = "data/shaders/animated_mesh_vs.glsl";
	shader_animation.fragment_source_path = "data/shaders/general_fs.glsl";

	shader_water.vertex_source_path = "data/shaders/water_vs.glsl";
	shader_water.fragment_source_path = "data/shaders/water_fs.glsl";

	shader_sky.vertex_source_path = "data/shaders/sky_vs.glsl";
	shader_sky.fragment_source_path = "data/shaders/sky_fs.glsl";
  
  shader_post_processing.vertex_source_path = "data/shaders/post_processing_vs.glsl";
	shader_post_processing.fragment_source_path = "data/shaders/post_processing_fs.glsl";
  
  shader_copy_output.vertex_source_path = "data/shaders/post_processing_vs.glsl";
	shader_copy_output.fragment_source_path = "data/shaders/copy_output_fs.glsl";
}


static void load_shaders()
{
	loadShader(shader);
	loadShader(shader_editor);
	loadShader(shader_solid);
	loadShader(shader_animation);
	loadShader(shader_water);
	loadShader(shader_sky);
  loadShader(shader_post_processing);
	loadShader(shader_copy_output);
}

static void reload_shaders()
{
	glUseProgram(0);
	glDeleteProgram(shader.ID);
	glDeleteProgram(shader_editor.ID);
	glDeleteProgram(shader_solid.ID);
	glDeleteProgram(shader_animation.ID);
	glDeleteProgram(shader_water.ID);
	glDeleteProgram(shader_sky.ID);
  glDeleteProgram(shader_post_processing.ID);
	glDeleteProgram(shader_copy_output.ID);

	load_shaders();
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE && mods == GLFW_MOD_SHIFT) && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		DEBUG_LOG("Close window requested.\n");
	}
	if (key == GLFW_KEY_W) 
	{
		keys[0] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_A)
	{
		keys[1] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_S)
	{
		keys[2] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_D)
	{
		keys[3] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_UP)
	{
		keys[4] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_LEFT)
	{
		keys[5] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_DOWN)
	{
		keys[6] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_RIGHT)
	{
		keys[7] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (key == GLFW_KEY_R && (action == GLFW_PRESS) && (mods &= GLFW_MOD_CONTROL))
	{
		DEBUG_LOG("RELOAD_SHADERS!\n");
		reload_shaders();
	}
	if (key == GLFW_KEY_TAB && (action == GLFW_PRESS))
	{
		if (!get_editor_state())
		{
			show_debug_panel = !show_debug_panel;
			if (show_debug_panel)
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool firstMouse = true;
	static float lastX = 0;
	static float lastY = 0;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float delta_yaw = xpos - lastX;
	float delta_pitch = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	delta_yaw *= sensitivity;
	delta_pitch *= sensitivity;

	if (show_debug_panel)
	{
		if (get_editor_state())
		{
			update_camera_orientation(camera_object_editor, delta_yaw, delta_pitch);
		}
		else
		{
			update_camera_orientation(camera_editor, delta_yaw, delta_pitch);
		}
	}
	else
	{
		update_camera_orientation(camera, delta_yaw, delta_pitch);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (ImGui::GetIO().WantCaptureMouse) 
	{
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		mouse_keys[0] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
		if (action == GLFW_PRESS)
		{
			if (show_debug_panel)
			{
				if (get_editor_state())
				{

				}
				else
				{
					double x_pos, y_pos;
					glfwGetCursorPos(window, &x_pos, &y_pos);

					double xn = x_pos / global_width * 2 - 1;
					double yn = -(y_pos / global_height * 2 - 1);

					Ray ray = get_ray(camera_editor, xn, yn);
					int index_out;
					if (ray_intersect_object_obb(ray, index_out))
					{
						DEBUG_LOG("Intersection: " << index_out << "\n");
						editor_select_object(index_out);
					}
					else
					{
						DEBUG_LOG("No intersection\n");
					}
				}
			}
		}
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		mouse_keys[1] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		mouse_keys[2] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
		if (show_debug_panel) {
			if (mouse_keys[2])
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else
			{
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	//glViewport(0, 0, width, height);
	global_width = width;
	global_height = height;
	recalculate_projection_matrix(camera, width, height);

	//@Note: this is very error prone. If the the camera data structure changes then this might fail. If statement to fall back to normal copy.
	if (offsetof(Camera, inv_proj) == offsetof(Camera, proj) + sizeof(Camera::proj))//Check that proj is right before inv_proj in Camera struct. Otherwise do normal copy.
	{
		char* copy_from_proj_pos = (char*)(&camera.proj);

		{//Copy to camera_object_eitor
			char* copy_to_proj_pos = (char*)(&camera_object_editor.proj);

			for (int i = 0; i < sizeof(Camera::proj) + sizeof(Camera::inv_proj); i++)
			{
				copy_to_proj_pos[i] = copy_from_proj_pos[i];
			}
		}

		{//Copy to camera_editor
			char* copy_to_proj_pos = (char*)(&camera_editor.proj);
			for (int i = 0; i < sizeof(Camera::proj) + sizeof(Camera::inv_proj); i++)
			{
				copy_to_proj_pos[i] = copy_from_proj_pos[i];
			}
		}
	}
	else
	{
		camera_object_editor.proj = camera.proj;
		camera_object_editor.inv_proj = camera.inv_proj;

		camera_editor.proj = camera.proj;
		camera_editor.inv_proj = camera.inv_proj;
	}

	DEBUG_LOG("Resize window. Width = " << global_width << " Height = " << global_height << "\n");

}

int main();

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR lpCmdLine, INT nCmdShow)
{
	return main();
}

int main()
{
	/* Initialize GLFW library */
	if (!glfwInit())
		return -1;

#if START_IN_FULLSCREEN 
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	#ifdef _DEBUG
		GLFWwindow* window = glfwCreateWindow(global_width, global_height, "GablingMarlow - DebugBuild", monitor, NULL);
	#else
		GLFWwindow* window = glfwCreateWindow(global_width, global_height, "GablingMarlow - ReleaseBuild", monitor, NULL);
	#endif

#else

	#ifdef _DEBUG
		GLFWwindow* window = glfwCreateWindow(global_width, global_height, "GablingMarlow - DebugBuild", NULL, NULL);
	#else
		GLFWwindow* window = glfwCreateWindow(global_width, global_height, "GablingMarlow - ReleaseBuild", NULL, NULL);
	#endif

#endif
	if (!window) {
		glfwTerminate();
		ERROR_LOG("Unable to create window\n");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#if START_IN_FULLSCREEN 
	global_width = mode->width;
	global_height = mode->height;
	glfwSetWindowMonitor(window, monitor, 0, 0, global_width, global_height, mode->refreshRate);
#endif


	//Activate V-sync
	glfwSwapInterval(1);


	setup_gl_renderer();

	setup_openal_audio();


	{
		GLint data;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &data);
		DEBUG_LOG("Max num texture units: " << data << "\n");
	}

	//Setup IMGUI
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	/*-----------------------------
				Readfile test
	-------------------------------*/
#ifdef TEST_READFILE
	test_readfile();
#endif // TEST_READFILE



#ifdef TEST_LOADOBJ
	test_loadobj();
#endif // TEST_LOADOBJ


#ifdef TEST_LOADDAE
	RawAnimMesh dae_global_rawmesh = { 0 };
	animation = {0};
	if (load_dae(TEST_DAE_FILE, &dae_global_rawmesh, &animation))
	{
		animation.mesh = upload_raw_anim_mesh(dae_global_rawmesh);
		delete[] dae_global_rawmesh.index_buffer;
		delete[] dae_global_rawmesh.vertex_buffer;
	}
#endif // TEST_LOADDAE
	
	setup_shaders();
	load_shaders();

	load_all_meshes();
	load_all_textures();

	load_world_from_file("data/world/testfile");
	

#ifdef FPS_TIMED
	int FPS = 0;
	double previousTime = glfwGetTime();
	int frameCount = 0;
#endif

	camera = get_default_camera(global_width, global_height);
	camera.yaw = 0.0f;
	camera.pitch = 15.0f;

	camera_editor = get_default_camera(global_width, global_height);
	camera_editor.position.x = 0.0f;
	camera_editor.position.y = 43.0f;
	camera_editor.position.z = 4.0f;

	camera_object_editor = get_default_camera(global_width, global_height);

	while (!glfwWindowShouldClose(window))
	{
#ifdef FPS_TIMED
		// Measure speed
		double currentTime = glfwGetTime();
		frameCount++;

		// If a second has passed.
		if (currentTime - previousTime >= 1.0)
		{
			//DEBUG_LOG(frameCount << "\n");
			FPS = frameCount;

			frameCount = 0;
			previousTime = currentTime;
		}
#endif

		// update other events like input handling 
		glfwPollEvents();
		glViewport(0, 0, 1920, 1080);
		//First pass
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		// clear the drawing surface
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Update
		if (show_debug_panel)
		{
			if (get_editor_state())
			{
				handle_editor_controlls(camera_object_editor);
			}
			else
			{
				update_world(camera_editor);
			}

		}
		else
		{
			update_world(camera);
		}

		//Draw
		start_next_frame();
		if (show_debug_panel)
		{
			if (get_editor_state())
			{
				render_editor_overlay(shader_editor, camera_object_editor);
				render_bounding_boxes(shader_solid, camera_object_editor);
			}
			else
			{
				render_sky(shader_sky, camera_editor);
				render_world(shader, camera_editor);
				render_world_water(shader_water, camera_editor);
				render_world_animations(shader_animation, camera_editor);
				render_editor_overlay(shader_editor, camera_editor);
				render_bounding_boxes(shader_solid, camera_editor);
			}
		}
		else
		{
			render_sky(shader_sky, camera);
			render_world(shader, camera);
			render_world_water(shader_water, camera);
			render_world_animations(shader_animation, camera);
		}

		glViewport(0, 0, 1920, 1080);
		// second pass
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_post); // back to default
			use_shader(shader_post_processing);
			glDisable(GL_DEPTH_TEST);
			glBindTexture(GL_TEXTURE_2D, texColorBuffer);
			glm::mat4 m = glm::mat4(1.0f);
			draw(get_plane_mesh(), m, m, m);
		}

		glViewport(0, 0, global_width, global_height);
		// third pass
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
			glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			use_shader(shader_copy_output);
			glBindTexture(GL_TEXTURE_2D, texColorBuffer_post);
			glDisable(GL_DEPTH_TEST);
			glm::mat4 m = glm::mat4(1.0f);
			draw(get_plane_mesh(), m, m, m);
		}


		if (show_debug_panel)
		{
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::Text("Debug Panel:");
			ImGui::Separator();
#ifdef FPS_TIMED
			ImGui::Value("FPS: ", FPS);
#endif

			//render_world_imgui_layer(camera);
			render_world_imgui_layer(camera_editor);


			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);

	}
	glfwTerminate();




	return 0;
}