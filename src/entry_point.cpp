

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


#include "Renderer/opengl_renderer.h"

#include "Renderer/obj_loader.h"


/*-----------------------------
			Globals
-------------------------------*/
static int global_height = 720;
static int global_width = 1280;

static void error_callback(int error, const char* description)
{
	ERROR_LOG(description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
		DEBUG_LOG("Close window requested.\n");
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	global_width = width;
	global_height = height;
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
	shader.vertex_source_path = "data/shaders/sample_vs.glsl";
	shader.fragment_source_path = "data/shaders/sample_fs.glsl";
	loadShader(shader);

	RawMesh raw_mesh_2 = load_obj_allocate_memory("data/models/dice_duplicate.obj");
	RawMesh raw_mesh_3 = load_obj_allocate_memory("data/models/dice.obj");
	RawMesh raw_mesh_4 = load_obj_allocate_memory("data/models/test_model_2.obj");
	RawMesh raw_mesh = load_obj_allocate_memory("data/models/test_model.obj");
	RawMesh raw_mesh_island = load_obj_allocate_memory("data/models/prototype_island.obj");
	
	Mesh mesh = upload_raw_mesh(raw_mesh);
	Mesh mesh_2 = upload_raw_mesh(raw_mesh_2);
	Mesh mesh_3 = upload_raw_mesh(raw_mesh_3);
	Mesh mesh_4 = upload_raw_mesh(raw_mesh_4);
	Mesh mesh_island = upload_raw_mesh(raw_mesh_island);
	
	delete[] raw_mesh.index_buffer;
	delete[] raw_mesh.vertex_buffer;
	delete[] raw_mesh_2.index_buffer;
	delete[] raw_mesh_2.vertex_buffer;
	delete[] raw_mesh_3.index_buffer;
	delete[] raw_mesh_3.vertex_buffer;
	delete[] raw_mesh_4.index_buffer;
	delete[] raw_mesh_4.vertex_buffer;
	delete[] raw_mesh_island.index_buffer;
	delete[] raw_mesh_island.vertex_buffer;

	
	

#ifdef FPS_TIMED
	int FPS = 0;
	double previousTime = glfwGetTime();
	int frameCount = 0;
#endif


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
		static float elapsed_time = 0;
		elapsed_time++;


		//Draw
		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::Text("Debug Panel:");
		ImGui::Separator();
#ifdef FPS_TIMED
		ImGui::Value("FPS: ", FPS);
#endif
		draw(mesh_island, shader, glm::vec3(0, -100, -400), 0, 200.0f);
		for (int i = 0; i < 10; i++)
		{
			draw(mesh, shader, glm::vec3(-1, 0, -1 - 5*i), elapsed_time * (1 + 1 * i), 0.7f);
			draw(mesh_2, shader, glm::vec3( 1, 0, -1 - 5 * i), elapsed_time * (1 + 2 * i), 0.4f);
			draw(mesh_3, shader, glm::vec3( 0,-1, -1 - 5 * i), elapsed_time * (1 + 3 * i), 0.2f);
			draw(mesh_4, shader, glm::vec3( 0, 1, -1 - 5 * i), elapsed_time * (1 + 4 * i), 0.6f);
		}

		ImGui::Render();

		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);

	}
	glfwTerminate();




	return 0;
}