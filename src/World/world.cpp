#include "world.h"
#include <Utils/readfile.h>
#include <Utils/writefile.h>
#include "Renderer/opengl_renderer.h"
#include "Renderer/obj_loader.h"
#include "glm/glm.hpp"
#include "imgui.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "globals.h"


local_scope int num_world_objects;
local_scope int render_amount = 15;
local_scope char* objects_data_buffer;

#define MESH_INDICES_OFFSET (0)
#define POSITIONS_OFFSET (MESH_INDICES_OFFSET + num_world_objects * sizeof(unsigned int))
#define SIZES_OFFSET (POSITIONS_OFFSET + num_world_objects * sizeof(glm::vec3))
#define ORIENTATIONS_OFFSET (SIZES_OFFSET + num_world_objects * sizeof(glm::vec3))

#define BUFFER_OBJECT_SIZE (sizeof(unsigned int) + sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4))

local_scope unsigned int* world_object_mesh_indices;
local_scope glm::vec3* world_object_positions;
local_scope glm::vec3* world_object_sizes;
local_scope glm::quat* world_object_orientations;

local_scope int ticks = 0;
local_scope float world_speed = 0.1f;
local_scope glm::vec4 global_light(-18.0f, 6.0f, 20.0f, 1.0f);




int get_num_world_objects() { return num_world_objects; }
glm::vec3* get_world_object_positions() { return world_object_positions; }
unsigned int* get_world_object_mesh_indices() { return world_object_mesh_indices; }




bool load_world_from_file(std::string world_filepath) { 
//#define GENERATE_SAMPLE_WORLD
#ifdef GENERATE_SAMPLE_WORLD

	num_world_objects = 100;
	objects_data_buffer = new char[num_world_objects * BUFFER_OBJECT_SIZE];

	world_object_mesh_indices = (unsigned int*)(objects_data_buffer + MESH_INDICES_OFFSET);
	world_object_positions = (glm::vec3*)(objects_data_buffer + POSITIONS_OFFSET);
	world_object_sizes = (glm::vec3*)(objects_data_buffer + SIZES_OFFSET);
	world_object_orientations = (glm::quat*)(objects_data_buffer + ORIENTATIONS_OFFSET);

	int num_meshes = get_num_meshes();
	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_mesh_indices[i] = i % num_meshes;
	}

	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_positions[i] = glm::vec3(((i % 10) / 3 - 1) * 2, ((i % 10) % 3 - 1) * 2, -2 - 2 * ((i / 9)));
	}

	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_sizes[i] = glm::vec3(1.0f);
	}

	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_orientations[i] = glm::quat(1, 0, 0, 0);
	}

	return true;

#else // GENERATE_SAMPLE_WORLD


	if (objects_data_buffer != nullptr)
	{
		delete[] objects_data_buffer;
	}

	int filesize = 0;
	get_filesize(world_filepath, &filesize);

	if (filesize > 0)
	{
		num_world_objects = filesize / BUFFER_OBJECT_SIZE;
		if (render_amount > num_world_objects)render_amount = num_world_objects;
		objects_data_buffer = new char[filesize];
		if (read_buffer(world_filepath, objects_data_buffer, filesize) != -1)
		{
			world_object_mesh_indices = (unsigned int*)(objects_data_buffer + MESH_INDICES_OFFSET);
			world_object_positions = (glm::vec3*)(objects_data_buffer + POSITIONS_OFFSET);
			world_object_sizes = (glm::vec3*)(objects_data_buffer + SIZES_OFFSET);
			world_object_orientations = (glm::quat*)(objects_data_buffer + ORIENTATIONS_OFFSET);
			return true;
		}
		else
		{
			return false;
		}
		
	}
	else
	{
		return false;
	}

#endif
}
bool save_world_to_file(std::string world_filepath)
{ 
	if (write_buffer_overwrite(world_filepath, objects_data_buffer, num_world_objects * BUFFER_OBJECT_SIZE) == -1)
	{
		return false;
	}
	else
	{
		return true;
	}
}


static int selected_object = 0;

void update_world()
{
	ticks++;
	if (keys[0] == true)// W
	{
		world_object_positions[selected_object].y += 0.05f;
	}
	if (keys[1] == true)// A
	{
		world_object_positions[selected_object].x -= 0.05f;
	}
	if (keys[2] == true)// S
	{
		world_object_positions[selected_object].y -= 0.05f;
	}
	if (keys[3] == true)// D
	{
		world_object_positions[selected_object].x += 0.05f;
	}



	if (keys[4] == true)// UP
	{
		//pitch++
		
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), camera_right);
		camera_facing = glm::vec3(rotateMat * glm::vec4(camera_facing, 1.0));
		camera_up = glm::vec3(rotateMat * glm::vec4(camera_up, 1.0));
	}
	if (keys[5] == true)// LEFT
	{
		//yaw
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), glm::radians(1.0f), glm::vec3(0, 1, 0));
		camera_facing = glm::vec3(rotateMat * glm::vec4(camera_facing, 1.0));
		camera_right = glm::vec3(rotateMat * glm::vec4(camera_right, 1.0));
	}
	if (keys[6] == true)// DOWN
	{
		//pitch--
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), -glm::radians(1.0f), camera_right);
		camera_facing = glm::vec3(rotateMat * glm::vec4(camera_facing, 1.0));
		camera_up = glm::vec3(rotateMat * glm::vec4(camera_up, 1.0));
	}
	if (keys[7] == true)// RIGHT
	{
		//yaw
		glm::mat4 rotateMat = glm::rotate(glm::mat4(1.0f), -glm::radians(1.0f), glm::vec3(0, 1, 0));
		camera_facing = glm::vec3(rotateMat * glm::vec4(camera_facing, 1.0));
		camera_right = glm::vec3(rotateMat * glm::vec4(camera_right, 1.0));
	}
}

void render_world(ShaderProgram &shader)
{

	ImGui::SliderFloat3("Global light", (float*)&global_light, -50.0f, 50.0f);
	ImGui::DragInt("Render objects count", &render_amount, 1, 0, num_world_objects);
	ImGui::Separator();
	ImGui::SliderFloat3("Camera position", (float*)&camera_position, -50.0f, 50.0f);

	//ImGui::SliderFloat("Time multiplicator", &world_speed, 0.0f, 2.0f);
	ImGui::Separator();
	ImGui::Text("Object modifiers");
	if (ImGui::Button("Select next object"))
	{
		selected_object = (selected_object + 1) % num_world_objects;
	}
	if (ImGui::Button("Select previous object"))
	{
		selected_object = (selected_object - 1) % num_world_objects;
	}
	ImGui::SliderFloat3("Position: ", (float*)&world_object_positions[selected_object], -50.0f, 50.0f);
	ImGui::SliderFloat3("Size: ", (float*)&world_object_sizes[selected_object], 0.0f, 100.0f);
	ImGui::SliderFloat4("Quaternion: ", (float*)&world_object_orientations[selected_object], -2.0f, 2.0f);
	world_object_orientations[selected_object] = glm::normalize(world_object_orientations[selected_object]);
	ImGui::Separator();
	if (ImGui::Button("Save world to testfile"))
	{
		save_world_to_file("data/world/testfile");
	}


	glUniform4fv(glGetUniformLocation(shader.ID, "global_light"), 1, (float*)&global_light);

	int num_meshes = get_num_meshes();
	if (num_meshes > 0)
	{
		Mesh* meshes = get_meshes();
		for (int i = 0; i < render_amount; i++)
		{
			unsigned int index = world_object_mesh_indices[i];
			if (index < num_meshes)
			{
				draw(meshes[index], shader, world_object_positions[i], world_object_sizes[i], world_object_orientations[i]);
			}
		}
	}


}