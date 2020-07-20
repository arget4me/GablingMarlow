

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

#include "World/world.h"
#include "World/Editor/world_editor.h"


/*-----------------------------
			Globals
-------------------------------*/
#define GLOBALS_DEFINITIONS
#include "globals.h"

local_scope Camera camera;
local_scope Camera camera_object_editor;

static void error_callback(int error, const char* description)
{
	ERROR_LOG(description);
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

	if (!get_editor_state())
		update_camera_orientation(camera, delta_yaw, delta_pitch);

	if (show_debug_panel)
	{
		if (get_editor_state())
			update_camera_orientation(camera_object_editor, delta_yaw, delta_pitch);
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

					Ray ray = get_ray(camera, xn, yn);
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
	glViewport(0, 0, width, height);
	global_width = width;
	global_height = height;
	recalculate_projection_matrix(camera, width, height);
	recalculate_projection_matrix(camera_object_editor, width, height);
	DEBUG_LOG("Resize window. Width = " << global_width << " Height = " << global_height << "\n");

}

int main(int argc, char* argv[])
{
	/* Initialize GLFW library */
	if (!glfwInit())
		return -1;
	
	GLFWwindow* window = glfwCreateWindow(global_width, global_height, "GablingMarlow", NULL, NULL);
	if (!window) {
		glfwTerminate();
		ERROR_LOG("Unable to create window\n");
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	//Activate V-sync
	glfwSwapInterval(1);


	setup_gl_renderer();


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

	
	ShaderProgram shader;
	shader.vertex_source_path = "data/shaders/general_vs.glsl";
	shader.fragment_source_path = "data/shaders/general_fs.glsl";


	ShaderProgram shader_editor;
	shader_editor.vertex_source_path = "data/shaders/general_vs.glsl";
	shader_editor.fragment_source_path = "data/shaders/editor_general_fs.glsl";

	ShaderProgram shader_solid;
	shader_solid.vertex_source_path = "data/shaders/solid_vs.glsl";
	shader_solid.fragment_source_path = "data/shaders/solid_fs.glsl";


	loadShader(shader);
	loadShader(shader_editor);
	loadShader(shader_solid);

	load_all_meshes();
	load_all_textures();

	load_world_from_file("data/world/testfile");
	

#ifdef FPS_TIMED
	int FPS = 0;
	double previousTime = glfwGetTime();
	int frameCount = 0;
#endif

	camera = get_default_camera(global_width, global_height);
	camera.position.x = 0.0f;
	camera.position.y = 43.0f;
	camera.position.z = 4.0f;
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

		

		// clear the drawing surface
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Update
		if (!get_editor_state())
			update_world(camera);
		if (show_debug_panel)
		{
			handle_editor_controlls(camera_object_editor);
		}


		//Draw
		if (!get_editor_state())
			render_world(shader, camera);
		if (show_debug_panel)
		{
			if (get_editor_state())
			{
				render_editor_overlay(shader_editor, camera_object_editor);
				render_bounding_boxes(shader_solid, camera_object_editor);
			}
			else
			{
				render_editor_overlay(shader_editor, camera);
				render_bounding_boxes(shader_solid, camera);
			}
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
			render_world_imgui_layer(camera);


			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);

	}
	glfwTerminate();




	return 0;
}