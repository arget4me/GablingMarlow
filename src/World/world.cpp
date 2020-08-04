#include "world.h"
#include <Utils/readfile.h>
#include <Utils/writefile.h>
#include <Utils/value_modifiers.h>
#include <Renderer/opengl_renderer.h>
#include <Renderer/obj_loader.h>
#include <glm/glm.hpp>
#include <imgui.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <World/Editor/world_editor.h>
#include <Renderer/animation_manager.h>

#include "globals.h"

local_scope const unsigned int num_world_objects = 2000;
local_scope unsigned int render_amount;
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

local_scope float world_speed = 0.1f;
local_scope glm::vec4 global_light(-200.0f, 300.0f, 100.0f, 1.0f);


local_scope glm::vec3 player_position(2.0f, 41.4, -3.0f);
local_scope glm::vec3 player_size(1.0f);
local_scope float player_yaw = 180.0f;
local_scope float player_movement_speed = 0.2f;
local_scope float player_rotation_speed = 2.5f;
local_scope glm::quat player_orientation(glm::vec3(0, glm::radians(player_yaw), 0));



int get_num_world_objects() { return num_world_objects; }
unsigned int& get_num_world_objects_rendered() { return render_amount; }
unsigned int* get_world_object_mesh_indices() { return world_object_mesh_indices; }
glm::vec3* get_world_object_positions() { return world_object_positions; }
glm::vec3* get_world_object_sizes() { return world_object_sizes; }
glm::quat* get_world_object_orientations() { return world_object_orientations; }
glm::vec3* get_global_light_position() { return (glm::vec3*)&global_light;  }

bool load_world_from_file(std::string world_filepath) { 
	if (objects_data_buffer != nullptr)
	{
		delete[] objects_data_buffer;
	}

	int filesize = 0;
	get_filesize(world_filepath, &filesize);

	if (filesize > 0)
	{
		render_amount = filesize / BUFFER_OBJECT_SIZE;
		objects_data_buffer = new char[num_world_objects * BUFFER_OBJECT_SIZE];


		world_object_mesh_indices = (unsigned int*)(objects_data_buffer + MESH_INDICES_OFFSET);
		world_object_positions = (glm::vec3*)(objects_data_buffer + POSITIONS_OFFSET);
		world_object_sizes = (glm::vec3*)(objects_data_buffer + SIZES_OFFSET);
		world_object_orientations = (glm::quat*)(objects_data_buffer + ORIENTATIONS_OFFSET);

		int indices_bytes = render_amount * sizeof(unsigned int);
		int positions_bytes = render_amount * sizeof(glm::vec3);
		int sizes_bytes = render_amount * sizeof(glm::vec3);
		int orientations_bytes = render_amount * sizeof(glm::quat);

		if (read_buffer(world_filepath, world_object_mesh_indices, render_amount * sizeof(unsigned int)) != -1)
		{
			if (read_buffer_offset(world_filepath, indices_bytes, world_object_positions, positions_bytes) != -1)
			{
				if (read_buffer_offset(world_filepath, indices_bytes + positions_bytes, world_object_sizes, sizes_bytes) != -1)
				{
					if (read_buffer_offset(world_filepath, indices_bytes + positions_bytes + sizes_bytes, world_object_orientations, orientations_bytes) != -1)
					{

						//@HACK: just to test terrain height adjustment
						for (int i = 0; i < render_amount; i++)
						{
							if (world_object_mesh_indices[i] == 4)
							{
								TerrainMap* terrain_map = get_terrain_map();
								terrain_map->scale = world_object_sizes[i];
								break;
							}
						}
						return true;
					}
				}
			}
		}
		return false;		
	}
	else
	{
		return false;
	}
}
bool save_world_to_file(std::string world_filepath)
{ 
	int indices_bytes = render_amount * sizeof(unsigned int);
	int positions_bytes = render_amount * sizeof(glm::vec3);
	int sizes_bytes = render_amount * sizeof(glm::vec3);
	int orientations_bytes = render_amount * sizeof(glm::quat);


	if (write_buffer_overwrite(world_filepath, world_object_mesh_indices, indices_bytes) != -1)
	{
		if (write_buffer_append(world_filepath, world_object_positions, positions_bytes) != -1)
		{
			if (write_buffer_append(world_filepath, world_object_sizes, sizes_bytes) != -1)
			{
				if (write_buffer_append(world_filepath, world_object_orientations, orientations_bytes) != -1)
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool ray_intersect_obb(Ray& ray, BoundingBox OBB_bounds, glm::vec3& position, glm::vec3& sizes, glm::quat& orientation, float& distance_out)
{

	glm::vec3 delta_position = position - ray.origin;

	glm::mat4 model_matrix_no_scale = glm::mat4(1.0f);
	model_matrix_no_scale = glm::translate(model_matrix_no_scale, position);
	model_matrix_no_scale = model_matrix_no_scale * glm::toMat4(orientation);
	OBB_bounds.min = OBB_bounds.min * sizes;
	OBB_bounds.max = OBB_bounds.max * sizes;

	float t_min = 0.0f;

	float t_max = 10000.0f;

	for (int i = 0; i < 3; i++)
	{
		glm::vec3 axis = glm::vec3(model_matrix_no_scale[i].x, model_matrix_no_scale[i].y, model_matrix_no_scale[i].z);

		float delta_axis_amount = glm::dot(delta_position, axis);
		float ray_axis_amount = glm::dot(ray.direction, axis);

		if (fabs(ray_axis_amount) > 0.001f)
		{
			float t1 = (delta_axis_amount + OBB_bounds.min[i]) / ray_axis_amount;
			float t2 = (delta_axis_amount + OBB_bounds.max[i]) / ray_axis_amount;

			if (t1 > t2)
			{
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			if (t1 > t_min)
			{
				t_min = t1;
			}

			if (t2 < t_max)
			{
				t_max = t2;
			}

			if (t_max < t_min)
			{
				return false;
			}
		}
		else
		{
			if (-delta_axis_amount + OBB_bounds.min[i] > 0.0f ||
				-delta_axis_amount + OBB_bounds.max[i] < 0.0f)
				return false;
		}
	}

	distance_out = t_min;
	return true;
}

bool ray_intersect_object_obb(Ray &ray, int& index_out)
{
	float distance = 1000000.0f;
	int closest_object_index = -1;

	for (int i = 0; i < render_amount; i++)
	{
		float temp_distance = -1.0f;
		if (ray_intersect_obb(ray, get_meshes_bounding_box()[world_object_mesh_indices[i]], world_object_positions[i], world_object_sizes[i], world_object_orientations[i], temp_distance))
		{
			if (temp_distance < distance)
			{
				closest_object_index = i;
				distance = temp_distance;
			}
		}
	}

	if (closest_object_index != -1)
	{
		index_out = closest_object_index;
		return true;
	}
	return false;
}

glm::vec3 barrycentric_position(glm::vec2 v1, glm::vec2 v2, glm::vec2 v3, glm::vec2 pos)
{
	float det = (v2.y - v3.y) * (v1.x - v3.x) + (v3.x - v2.x) * (v1.y - v3.y);
	float l1 = ((v2.y - v3.y) * (pos.x - v3.x) + (v3.x - v2.x) * (pos.y - v3.y)) / det;
	float l2 = ((v3.y - v1.y) * (pos.x - v3.x) + (v1.x - v3.x) * (pos.y - v3.y)) / det;
	float l3 = 1.0f - l1 - l2;
	return { l1, l2, l3 };
}

float get_terrain_height(glm::vec3 position)
{
	TerrainMap* terrain_map = get_terrain_map();
	//@Todo: subtract terrain_map position

	float grid_width = (*terrain_map).grid_width * (*terrain_map).scale.x;
	float grid_height = (*terrain_map).grid_height * (*terrain_map).scale.z;

	float size_width = grid_width * ((*terrain_map).terrain_width - 1);
	float size_height = grid_height * ((*terrain_map).terrain_height - 1);

	glm::vec3 adjusted_pos = position + glm::vec3(size_width / 2, 0, size_height / 2);

	if (adjusted_pos.x < 0 || adjusted_pos.x >= size_width || adjusted_pos.z < 0 || adjusted_pos.z >= size_height)
	{
		return 0.0f;
	}

	int x = floor(adjusted_pos.x / grid_width);
	int z = floor(adjusted_pos.z / grid_height);

	float x_pos = adjusted_pos.x / grid_width - x;
	float z_pos = adjusted_pos.z / grid_height - z;
	glm::vec2 pos(x_pos, z_pos);


	float terrain_height = 0;
	float h1 = 0;
	float h2 = 0;
	float h3 = 0;
	glm::vec3 coords;
	if (x > 1.0 - z)
	{
		coords = barrycentric_position({ 1, 0 }, { 1, 1 }, { 0, 1 }, pos);
		h1 = terrain_map->height_values[(x + 1) + z * (*terrain_map).terrain_width];
		h2 = terrain_map->height_values[(x + 1) + (z + 1) * (*terrain_map).terrain_width];
		h3 = terrain_map->height_values[x + (z + 1) * (*terrain_map).terrain_width];
	}
	else
	{
		coords = barrycentric_position({ 0, 0 }, { 1, 0 }, { 0, 1 }, pos);
		h1 = terrain_map->height_values[x + z * (*terrain_map).terrain_width];
		h2 = terrain_map->height_values[(x + 1) + z * (*terrain_map).terrain_width];
		h3 = terrain_map->height_values[x + (z + 1) * (*terrain_map).terrain_width];
	}

	terrain_height = ((coords.x * h1) + (coords.y * h2) + (coords.z * h3)) * (*terrain_map).scale.y;

	return terrain_height;

}

void update_world(Camera &camera)
{
	if (!show_debug_panel)
	{
		if ((keys[0] == true) || (keys[4] == true))// W, UP
		{
			player_position += player_movement_speed * glm::vec3(sin(glm::radians(player_yaw)), 0.0f, cos(glm::radians(player_yaw)));
		}
		if ((keys[1] == true) || (keys[5] == true))// A, LEFT
		{
			player_yaw += player_rotation_speed; //player_position -= player_movement_speed * camera.right;
			player_orientation = glm::quat(glm::vec3(0, glm::radians(player_yaw),0));

			camera.yaw -= player_rotation_speed;
		}
		if ((keys[2] == true) || (keys[6] == true))// S, DOWN
		{
			player_position -= player_movement_speed * glm::vec3(sin(glm::radians(player_yaw)), 0.0f, cos(glm::radians(player_yaw)));
		}
		if ((keys[3] == true) || (keys[7] == true))// D, RIGHT
		{
			player_yaw -= player_rotation_speed;//player_position += player_movement_speed * camera.right;
			player_orientation = glm::quat(glm::vec3(0, glm::radians(player_yaw), 0));
			camera.yaw += player_rotation_speed;
		}
		player_position.y = get_terrain_height(player_position);
		
		update_camera(camera, &player_position);
	}
	else
	{
		handle_editor_controlls(camera);
	}
	static float time = 0.0f;

	update_animation(animation, time);

	time += 1.0f / 61.0f;
}

void render_world(ShaderProgram &shader, Camera& camera)
{
	glm::mat4 view_matrix = get_view_matrix(camera);

	use_shader(shader);

	glUniform4fv(glGetUniformLocation(shader.ID, "global_light"), 1, (float*)&global_light);

	int num_meshes = get_num_meshes();
	if (num_meshes > 0)
	{
		Mesh* meshes = get_meshes();
		for (int i = 0; i < render_amount; i++)
		{
			unsigned int index = world_object_mesh_indices[i];
			if (index == 2)continue;
			if (index < num_meshes)
			{
				glm::mat4 model_matrix = glm::mat4(1.0f);
				model_matrix = glm::translate(model_matrix, world_object_positions[i]);
				model_matrix = model_matrix * glm::toMat4(world_object_orientations[i]);
				model_matrix = glm::scale(model_matrix, world_object_sizes[i]);
				draw(meshes[index], model_matrix, view_matrix, camera.proj);



			}
		}
	}
	
}

void render_world_animations(ShaderProgram& shader, Camera& camera)
{
	glm::mat4 view_matrix = get_view_matrix(camera);

	use_shader(shader);

	glUniform4fv(glGetUniformLocation(shader.ID, "global_light"), 1, (float*)&global_light);

	int num_uploaded_transforms = animation.num_bones;
	if (num_uploaded_transforms > 100)num_uploaded_transforms = 100;
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "bone_transforms"), num_uploaded_transforms, GL_FALSE, (float*)animation.animation_transforms);
	int num_meshes = get_num_meshes();
	if (num_meshes > 0)
	{
		Mesh* meshes = get_meshes();
		for (int i = 0; i < render_amount; i++)
		{
			unsigned int index = world_object_mesh_indices[i];
			if (index != 2)continue;
			if (index < num_meshes)
			{
				glm::mat4 model_matrix = glm::mat4(1.0f);
				model_matrix = glm::translate(model_matrix, world_object_positions[i]);
				model_matrix = model_matrix * glm::toMat4(world_object_orientations[i]);
				model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
				model_matrix = glm::scale(model_matrix, world_object_sizes[i]);
				draw(animation.mesh, model_matrix, view_matrix, camera.proj);



			}
		}
	}

	{
		glm::mat4 model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, player_position);
		model_matrix = model_matrix * glm::toMat4(player_orientation);
		model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model_matrix = glm::scale(model_matrix, player_size);
		draw(animation.mesh, model_matrix, view_matrix, camera.proj);
	}
}