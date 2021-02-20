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

#define STRUCTURED_BINARY_IO_IMPLEMENTATION
#include "Utils/structured_binary/structured_binary_io.h"
#include "Utils/structured_binary/structured_binary_wrapper.h"

#define STRUCTURED_BINARY_IMGUI_INTEGRATION_IMPLEMENTATION
#include "Utils/structured_binary/structured_binary_imgui_integration.h"

#include "Renderer/opengl_renderer.h"

#include "Renderer/obj_loader.h"
#include "Renderer/dae_loader.h"

#include "World/world.h"
#include "World/Editor/world_editor.h"




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

void preframe_setup_post_processing()
{
	//First pass
	glViewport(0, 0, 1920, 1080);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST);
}

void post_processing()
{
	// second pass
	{
		glViewport(0, 0, global_width, global_height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
		glDisable(GL_DEPTH_TEST);

		use_shader(shader_post_processing);
		static GLuint location_render_style = glGetUniformLocation(shader_post_processing.ID, "render_style");
		glUniform1i(location_render_style, global_render_style);
		static GLuint location_render_outlines = glGetUniformLocation(shader_post_processing.ID, "render_outlines");
		glUniform1i(location_render_outlines, global_render_outlines);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);

		glm::mat4 m = glm::mat4(1.0f);
		draw(get_plane_mesh(), m, m, m);
	}
}

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
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float delta_yaw = (float)xpos - lastX;
	float delta_pitch = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

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
	if (ImGui::GetIO().WantCaptureMouse && show_debug_panel)
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

					Ray ray = get_ray(camera_editor, (float)xn, (float)yn);
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
	global_width = width;
	global_height = height;

	recalculate_projection_matrix(camera, (float)width, (float)height);

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

inline LARGE_INTEGER high_presission_time()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result;
}

inline double get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER stop, LARGE_INTEGER frequency)
{
	double elapsed = ((double)(stop.QuadPart - start.QuadPart)) / (double)(frequency.QuadPart);
	return elapsed;
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
	

#ifdef _DEBUG
	const char* WINDOW_TITLE = "GablingMarlow - DebugBuild";
#else
	const char* WINDOW_TITLE = "GablingMarlow - ReleaseBuild";
#endif


#if START_IN_FULLSCREEN
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	global_width = mode->width;
	global_height = mode->height;
	GLFWwindow* window = glfwCreateWindow(global_width, global_height, WINDOW_TITLE, monitor, NULL);
#else
	GLFWwindow* window = glfwCreateWindow(global_width, global_height, WINDOW_TITLE, NULL, NULL);
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


//<-----Setup for fixed framerate-----

	//Query what refreshrate the monitior is set to.
	DEVMODE lpDevMode;
	memset(&lpDevMode, 0, sizeof(DEVMODE));
	lpDevMode.dmSize = sizeof(DEVMODE);
	lpDevMode.dmDriverExtra = 0;
	unsigned int monitorHz;
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &lpDevMode)) {
		ERROR_LOG("Cannot retrieve monitor framerate!\n");
		monitorHz = 60;
	}
	else
	{
		DEBUG_LOG("Monitor refresh rate is " << lpDevMode.dmDisplayFrequency << "Hz\n");
		monitorHz = lpDevMode.dmDisplayFrequency;
	}
	const float fixed_frame_time_seconds = 1.0f / (float)monitorHz;

	//Force the OS scheduler operate at 1ms. (1ms is the lowest we can set it to..)
	uint32_t SchedulerGrandularity1MS = 1;
	bool sleep_granularity_set = false;;
	{
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		{
			// Error; application can't continue.
		}
		if (tc.wPeriodMin <= SchedulerGrandularity1MS)
		{
			SchedulerGrandularity1MS = tc.wPeriodMin;
			if (sleep_granularity_set = (timeBeginPeriod(SchedulerGrandularity1MS) == TIMERR_NOERROR))
			{
				DEBUG_LOG("Scheduler granularity set to 1ms, for sleep to work properly.\n");
			};
		}

	}
	const float constant_sleep_padding = (float)SchedulerGrandularity1MS * 1.5f;

	//Set thread priority to the highest value to ensure, even though the scheduler granularity is 1ms, that there aren't frame misses due to ordering of thread priority.
	HANDLE hProcess = GetCurrentProcess();
	if (!SetPriorityClass(
		hProcess,
		REALTIME_PRIORITY_CLASS
	))
	{
		ERROR_LOG("Can't set thread priority class\n");
	}
	else
	{
		DWORD process_priority = GetPriorityClass(hProcess);
		DEBUG_LOG("Thread priority set to: ");
		switch (process_priority)
		{
		case ABOVE_NORMAL_PRIORITY_CLASS:
		{
			DEBUG_LOG("ABOVE_NORMAL_PRIORITY_CLASS" << "\n");
		}break;
		case BELOW_NORMAL_PRIORITY_CLASS:
		{
			DEBUG_LOG("BELOW_NORMAL_PRIORITY_CLASS" << "\n");
		}break;
		case HIGH_PRIORITY_CLASS:
		{
			DEBUG_LOG("HIGH_PRIORITY_CLASS" << "\n");
		}break;
		case IDLE_PRIORITY_CLASS:
		{
			DEBUG_LOG("IDLE_PRIORITY_CLASS" << "\n");
		}break;
		case NORMAL_PRIORITY_CLASS:
		{
			DEBUG_LOG("NORMAL_PRIORITY_CLASS" << "\n");
		}break;
		case REALTIME_PRIORITY_CLASS:
		{
			DEBUG_LOG("REALTIME_PRIORITY_CLASS" << "\n");
		}break;
		}
	}

	//Setup the timer frequency to be able to measure high accuracy time properly. 1us accuracy.
	LARGE_INTEGER system_timer_frequency;
	if (!QueryPerformanceFrequency(&system_timer_frequency))
	{
		ERROR_LOG("[QueryPerformanceFrequency(&system_timer_frequency)] Can't aquire system timer frequecy!");
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

#if VSYNC_ON
	//Activate V-sync. Sync the render output to the monitor refresh signal.
	glfwSwapInterval(1);
#endif
//-----Setup for fixed framerate----->//

	setup_gl_renderer();

	//Check max number of texture units avaliable on the GPU
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

#ifdef TEST_READFILE
	/*-----------------------------
				Readfile test
	-------------------------------*/
	test_readfile();
#endif

#ifdef TEST_LOADOBJ
	test_loadobj();
#endif

#ifdef TEST_LOADDAE //@Hack: Player uses this model for now, change this later to the proper animation models system-
	RawAnimMesh dae_global_rawmesh = { 0 };
	animation = {0};
	if (load_dae(TEST_DAE_FILE, &dae_global_rawmesh, &animation))
	{
		animation.mesh = upload_raw_anim_mesh(dae_global_rawmesh);
		delete[] dae_global_rawmesh.index_buffer;
		delete[] dae_global_rawmesh.vertex_buffer;
	}
#endif
	
	setup_shaders();
	load_shaders();

	load_all_meshes();
	load_all_textures();

	//Fill structured binary - with worldfile meta data
	{
		DWORD nBufferLength = 0;
		LPTSTR WORLD_FOLDER = NULL;
		nBufferLength = GetCurrentDirectory(
			nBufferLength,
			WORLD_FOLDER
		);

		const char* DIR = "\\data\\world\\*";
		constexpr unsigned long DIR_STRING_SIZE = sizeof("\\data\\world\\*");
		unsigned long buffer_size = (nBufferLength + DIR_STRING_SIZE);

		WORLD_FOLDER = new char[buffer_size];
		nBufferLength = GetCurrentDirectory(
			nBufferLength,
			WORLD_FOLDER
		);

		for (int i = 0; i < DIR_STRING_SIZE; i++)
		{
			WORLD_FOLDER[nBufferLength + i] = DIR[i];
		}

		STRUCTURED_IO::StructuredData* world_meta_data = STRUCTURED_IO::create_new_structured_data("Worldfiles meta data");

		WIN32_FIND_DATA ffd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind = FindFirstFile(WORLD_FOLDER, &ffd);
		int num_files = 0;
		if (INVALID_HANDLE_VALUE != hFind)
		{
			num_files++;
			while (FindNextFile(hFind, &ffd) != 0)
			{
				num_files++;
			}
		}
		FindClose(hFind);


		constexpr int CHAR_STRING_SIZE = sizeof(ffd.cFileName); //char[260]

		//Ignore . and  .. that appear first in each folder
		if (num_files > 2)
		{
			const int num_map_files = num_files - 2; 
			world_meta_data->value = STRUCTURED_IO::add_list_value(num_map_files);
			STRUCTURED_IO::StructuredDataList* world_list = STRUCTURED_IO::get_list_from_structure(world_meta_data->value);

			hFind = FindFirstFile(WORLD_FOLDER, &ffd);
			for (int i = 0; i < num_files; i++)
			{
				if(i >= 2)
				{
					int index = i - 2;
					STRUCTURED_IO::add_value_to_list(world_list, STRUCTURED_IO::add_text_value(ffd.cFileName, CHAR_STRING_SIZE), index);
				}
				FindNextFile(hFind, &ffd);
			}
			FindClose(hFind);

			global_structured_data = world_meta_data;
		}

		delete[] WORLD_FOLDER;

#if 0
		//test save
		{
			int size_of_structure = 0;
			STRUCTURED_IO::get_size_bytes_structured_binary(size_of_structure, global_structured_data);

			if (size_of_structure > 0)
			{
				char* save_buffer = new char[size_of_structure];
				int token_index = 0;
				if (STRUCTURED_IO::write_structured_binary(token_index, save_buffer, size_of_structure, global_structured_data))
				{
					DEBUG_LOG("Saved the structured data");
					STRUCTURED_IO::destroy_structured_data(global_structured_data);
					{

						DEBUG_LOG("Deleted structured data");
						token_index = 0;
						if (STRUCTURED_IO::parse_structured_binary(token_index, save_buffer, size_of_structure, &global_structured_data))
						{

							DEBUG_LOG("Parsed the structured data");
						}
					}
				}
				delete[] save_buffer;


			}
		
		}
#endif
	}

	load_world_from_file(WORLD_FILE_PATH);

	camera = get_default_camera((float)global_width, (float)global_height);
	camera.yaw = 0.0f;
	camera.pitch = 15.0f;

	camera_editor = get_default_camera((float)global_width, (float)global_height);
	camera_editor.position.x = 0.0f;
	camera_editor.position.y = 43.0f;
	camera_editor.position.z = 4.0f;

	camera_object_editor = get_default_camera((float)global_width, (float)global_height);

	// Measure speed
#ifdef FPS_TIMED //If the fps counter should be displayed in the IMGUI debug window. (note that the framerate is always fixed, this is just if it should be displayed)
	int FPS = 0;
	LARGE_INTEGER previous_time = high_presission_time();
	int frameCount = 0;
#endif
	LARGE_INTEGER frame_start_time = high_presission_time();
	glfwSwapBuffers(window);
	double render_buffer_swap_time = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);
	while (!glfwWindowShouldClose(window))
	{
		
#ifdef FPS_TIMED
		// If a second has passed.
		if (get_elapsed_time(previous_time, high_presission_time(), system_timer_frequency) >= 1.0)
		{
			FPS = frameCount;
			frameCount = 0;
			previous_time = high_presission_time();
		}
		frameCount++;
#endif

		// update other events like input handling 
		glfwPollEvents();

		if (global_do_post_processing)
		{
			preframe_setup_post_processing();
		}
		else
		{
			glViewport(0, 0, global_width, global_height);
		}

		// clear the drawing surface
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		//Update
		if (show_debug_panel)
		{
			if (get_editor_state())
			{
				handle_editor_controlls(camera_object_editor, fixed_frame_time_seconds);
			}
			else
			{
				update_world(camera_editor, fixed_frame_time_seconds);
			}

		}
		else
		{
			update_world(camera, fixed_frame_time_seconds);
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

		if (global_do_post_processing)
		{
			post_processing();
		}


#define RUN_IMGUI 1
#if RUN_IMGUI
		if (show_debug_panel)
		{
			ImGui_ImplGlfwGL3_NewFrame();
			ImGui::Text("Debug Panel:");
			ImGui::Separator();
#ifdef FPS_TIMED
			ImGui::Value("FPS", FPS);
#endif

			//render_world_imgui_layer(camera);
			render_world_imgui_layer(camera_editor);


			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		}
#endif

		

		//Enforce the constant framerate. 1.0/monitorHz seconds per frame.
		{
#define DISPLAY_FRAMETIME_DEBUG 0
#define DISPLAY_MISSED_FRAMETIME_DEBUG 1
#if !DISPLAY_FRAMETIME_DEBUG
			double frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);
			int padded_ms_left = 0;
			if (frame_elapsed_time_seconds < fixed_frame_time_seconds)
			{
				padded_ms_left = (int)(1000.0f * (fixed_frame_time_seconds - frame_elapsed_time_seconds) - constant_sleep_padding);
				if (sleep_granularity_set && padded_ms_left > 0)
				{

					DWORD SleepMs = (DWORD)padded_ms_left;
					Sleep(SleepMs);

				}

				frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);

				while (fixed_frame_time_seconds - frame_elapsed_time_seconds > 1e-6f)
				{
					frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);
				}
			}
			#if DISPLAY_MISSED_FRAMETIME_DEBUG
			else
			{

				ERROR_LOG("Missed a frame! Frametime = " << 1000.0f * frame_elapsed_time_seconds << "\tRenderbuffer_swap_time: "<< 1000.0f * render_buffer_swap_time << " \n");
			}
			#endif
			frame_start_time = high_presission_time();

#else DISPLAY_FRAMETIME_DEBUG
			float frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);
			int padded_ms_left = 0;
			DWORD StartSleepMs = 0;
			float actual_sleep_time = 0;
			float initial_sleep_time = 1000.0f * (fixed_frame_time_seconds - frame_elapsed_time_seconds);
			LARGE_INTEGER start_wait_timer = high_presission_time();
			LARGE_INTEGER stop_wait_timer;
			LARGE_INTEGER start_spinlock_timer = start_wait_timer;
			if (frame_elapsed_time_seconds < fixed_frame_time_seconds)
			{
				padded_ms_left = (int)(1000.0f * (fixed_frame_time_seconds - frame_elapsed_time_seconds) - constant_sleep_padding);
				if (sleep_granularity_set && padded_ms_left > 0)
				{
					
					DWORD SleepMs = (DWORD)padded_ms_left;
					LARGE_INTEGER sleep_start = high_presission_time();
					Sleep(SleepMs);
					actual_sleep_time = 1000.0f * get_elapsed_time(sleep_start, high_presission_time(), system_timer_frequency);
					StartSleepMs = SleepMs;

				}

				start_spinlock_timer = high_presission_time();
				frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, start_spinlock_timer, system_timer_frequency);

				while (fixed_frame_time_seconds - frame_elapsed_time_seconds > 1e-6f )
				{
					frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);
				}
			}

			stop_wait_timer = high_presission_time();
			frame_elapsed_time_seconds = get_elapsed_time(frame_start_time, stop_wait_timer, system_timer_frequency);
			frame_start_time = stop_wait_timer;
#if DISPLAY_MISSED_FRAMETIME_DEBUG
			if (initial_sleep_time > 0.0f)
#endif
			{
				float spinlock_time_total = 1000.0f * get_elapsed_time(start_spinlock_timer, stop_wait_timer, system_timer_frequency);
				float wait_time_total = 1000.0f * get_elapsed_time(start_wait_timer, stop_wait_timer, system_timer_frequency);
				DEBUG_LOG(1000.0f * frame_elapsed_time_seconds << "\t[ " << initial_sleep_time << " : " << actual_sleep_time << " : " << spinlock_time_total << " ]\t" << wait_time_total << "\n");
			}
#endif
		}
		glfwSwapBuffers(window);
		render_buffer_swap_time = get_elapsed_time(frame_start_time, high_presission_time(), system_timer_frequency);


	}
	glfwTerminate();
	timeEndPeriod(SchedulerGrandularity1MS);


	return 0;
}