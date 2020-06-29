#include "world.h"
#include "Utils/readfile.h"
#include "Renderer/opengl_renderer.h"
#include "Renderer/obj_loader.h"
#include "glm/glm.hpp"



static int num_world_objects;
static glm::vec3* world_object_positions;
static glm::vec3* world_object_sizes;

static unsigned int* world_object_mesh_indices;



int get_num_world_objects() { return num_world_objects; }
glm::vec3* get_world_object_positions() { return world_object_positions; }
unsigned int* get_world_object_mesh_indices() { return world_object_mesh_indices; }




bool load_world_from_file(std::string world_filepath) { 
	
	num_world_objects = 10;
	world_object_positions = new glm::vec3[num_world_objects];
	world_object_sizes = new glm::vec3[num_world_objects];
	world_object_mesh_indices = new unsigned int[num_world_objects];

	int num_meshes = get_num_meshes();
	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_mesh_indices[i] = i % num_meshes;
	}

	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_positions[i] = glm::vec3((i / 3 - 1) * 2, (i % 3 - 1) * 2, -5 - 2 * ((i/3) % 3));
	}

	for (int i = 0; i < num_world_objects; i++)
	{
		world_object_sizes[i] = glm::vec3(0.5f + i*0.25f);
	}

	return true;
}
bool save_world_to_file(std::string world_filepath) { return true; }

int ticks = 0;

void update_world()
{
	ticks++;
}

void render_world(ShaderProgram &shader)
{
	int num_meshes = get_num_meshes();
	if (num_meshes > 0)
	{
		Mesh* meshes = get_meshes();
		for (int i = 0; i < num_world_objects; i++)
		{
			unsigned int index = world_object_mesh_indices[i];
			if (index < num_meshes)
			{
				draw(meshes[index], shader, world_object_positions[i], ticks * 0.1f, world_object_sizes[i]);
			}
		}
	}
}