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

local_scope glm::vec4 global_light(-200.0f, 300.0f, 100.0f, 1.0f);


local_scope glm::vec3 player_position(2.0f, 41.4, -3.0f);
local_scope glm::vec3 player_size(1.0f);
local_scope float player_yaw = 180.0f;
local_scope float player_movement_speed = 12.0f; // units/s
local_scope float player_rotation_speed = 150.0f;
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
						{//@HACK here
							for (int i = 0; i < render_amount; i++)
							{
								if (world_object_mesh_indices[i] == 4)
								{
									//@HACK: just to adjust terrain height with object scale. 
									//Change to terrain scale and render the terrain with that scale.
									TerrainMap* terrain_map = get_terrain_map();
									terrain_map->scale = world_object_sizes[i];
									break;
								}
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
	//@Todo: subtract terrain_map position. Same as scale add position to terrain map. 

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

void update_world(Camera &camera, float deltaTime)
{
	if (!show_debug_panel)
	{
		if ((keys[0] == true) || (keys[4] == true))// W, UP
		{
			player_position += player_movement_speed * glm::vec3(sin(glm::radians(player_yaw)), 0.0f, cos(glm::radians(player_yaw))) * deltaTime;
		}
		if ((keys[1] == true) || (keys[5] == true))// A, LEFT
		{
			float rotation_amount = player_rotation_speed * deltaTime;
			player_yaw += rotation_amount;
			player_orientation = glm::quat(glm::vec3(0, glm::radians(player_yaw),0));

			camera.yaw -= rotation_amount;
		}
		if ((keys[2] == true) || (keys[6] == true))// S, DOWN
		{
			player_position -= player_movement_speed * glm::vec3(sin(glm::radians(player_yaw)), 0.0f, cos(glm::radians(player_yaw))) * deltaTime;
		}
		if ((keys[3] == true) || (keys[7] == true))// D, RIGHT
		{
			float rotation_amount = player_rotation_speed * deltaTime;
			player_yaw -= rotation_amount;
			player_orientation = glm::quat(glm::vec3(0, glm::radians(player_yaw), 0));

			camera.yaw += rotation_amount;
		}
		player_position.y = get_terrain_height(player_position);
		if (!mouse_keys[2])
		{
			float interpolate_speed = fabs(camera.yaw - -(player_yaw - 180)) * 6.0f * deltaTime;
			interpolate_float(camera.yaw, interpolate_speed, -(player_yaw-180.0f));
		}
		
		update_camera_follow(camera, &player_position);
	}
	else
	{
		handle_editor_controlls(camera, deltaTime);
	}
	

	update_animation(animation, game_time);
	game_time += deltaTime;
	
	float scroll_speed = 0.03f * deltaTime;
	float displacement_speed = 0.02f * deltaTime;
	loop_float(offsets[0], -displacement_speed, 0.0f, 1.0f);
	loop_float(offsets[1], -displacement_speed, 0.0f, 1.0f);
	loop_float(offsets[2], scroll_speed, 0.0f, 1.0f);
	loop_float(offsets[3], scroll_speed, 0.0f, 1.0f);
}


void render_sky(ShaderProgram& shader, Camera& camera)
{
	//@TODO: look at better skybox setup
	glm::mat4 view_matrix = get_view_matrix(camera);
	use_shader(shader);

	static GLuint location_sky_color = glGetUniformLocation(shader.ID, "sky_color");
	glUniform4fv(location_sky_color, 1, &color_dark[0]);


	glm::mat4 model_matrix(1.0f);
	model_matrix = glm::translate(model_matrix, glm::vec3(camera.position.x, 0, camera.position.z));
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 right = glm::cross(camera.dir, up);
	glm::vec3 dir = glm::cross(up, right);
	model_matrix[2] = glm::vec4(right.x, right.y, right.z, 0);
	model_matrix[0] = glm::vec4(dir.x, dir.y, dir.z, 0);
	model_matrix[1] = glm::vec4(up.x, up.y, up.z, 0);

	model_matrix = glm::scale(model_matrix, glm::vec3(500, 300, 500));

	glCullFace(GL_FRONT);
	draw(get_cube_mesh(), model_matrix, view_matrix, camera.proj);
	glCullFace(GL_BACK);
}

void render_world(ShaderProgram &shader, Camera& camera)
{
	glm::mat4 view_matrix = get_view_matrix(camera);

	use_shader(shader);

	static GLuint location_global_light = glGetUniformLocation(shader.ID, "global_light");
	glUniform4fv(location_global_light, 1, (float*)&global_light);

	static GLuint location_sky_color = glGetUniformLocation(shader.ID, "sky_color");
	glUniform4fv(location_sky_color, 1, &color_dark[0]);

	int num_meshes = get_num_meshes();
	if (num_meshes > 0)
	{
		Mesh* meshes = get_meshes();
		for (int i = 0; i < render_amount; i++)
		{
			unsigned int index = world_object_mesh_indices[i];
			if (index < num_meshes)
			{
				//@TODO: Would probably be better to use a mesh-tag or similar. Then it can be sorted on the tag when a render queue is implemented.
				if (index == 5)glDisable(GL_CULL_FACE);
				glm::mat4 model_matrix = glm::mat4(1.0f);
				model_matrix = glm::translate(model_matrix, world_object_positions[i]);
				model_matrix = model_matrix * glm::toMat4(world_object_orientations[i]);
				model_matrix = glm::scale(model_matrix, world_object_sizes[i]);
				set_texture(get_textures()[index]);
				draw(meshes[index], model_matrix, view_matrix, camera.proj);
				if (index == 5)glEnable(GL_CULL_FACE);
			}
		}
	}
	
}

void render_world_water(ShaderProgram& shader, Camera& camera)
{
	glm::mat4 view_matrix = get_view_matrix(camera);

	use_shader(shader);
	static GLuint location_displacement_offset = glGetUniformLocation(shader.ID, "displacement_offset");
	static GLuint location_mask_offset = glGetUniformLocation(shader.ID, "mask_offset");
	static GLuint location_color_dark = glGetUniformLocation(shader.ID, "color_dark");
	static GLuint location_color_mid = glGetUniformLocation(shader.ID, "color_mid");
	static GLuint location_color_light = glGetUniformLocation(shader.ID, "color_light");
	static GLuint location_water_scroll = glGetUniformLocation(shader.ID, "water_scroll");


	glUniform2fv(location_displacement_offset, 1, &offsets[0]);
	glUniform2fv(location_mask_offset, 1, &offsets[2]);
	glUniform4fv(location_color_dark, 1, &color_dark[0]);
	glUniform4fv(location_color_mid, 1, &color_mid[0]);
	glUniform4fv(location_color_light, 1, &color_light[0]);
	glUniform1f(location_water_scroll, game_time);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, get_textures()[get_num_meshes() + 1]);
	set_texture(get_textures()[get_num_meshes()]); 
	{
		glm::mat4 model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, glm::vec3(0));
		model_matrix = glm::scale(model_matrix, glm::vec3(20));
		draw(get_water_mesh(), model_matrix, view_matrix, camera.proj);
	}
}

void render_world_animations(ShaderProgram& shader, Camera& camera)
{
	glm::mat4 view_matrix = get_view_matrix(camera);

	use_shader(shader);

	static GLuint location_global_light = glGetUniformLocation(shader.ID, "global_light");
	glUniform4fv(location_global_light, 1, (float*)&global_light);

	static GLuint location_sky_color = glGetUniformLocation(shader.ID, "sky_color");
	glUniform4fv(location_sky_color, 1, &color_dark[0]);

	int num_uploaded_transforms = animation.num_bones;
	if (num_uploaded_transforms > 100)num_uploaded_transforms = 100;
	static GLuint location_bone_transforms = glGetUniformLocation(shader.ID, "bone_transforms");
	glUniformMatrix4fv(location_bone_transforms, num_uploaded_transforms, GL_FALSE, (float*)animation.animation_transforms);

	{
		glm::mat4 model_matrix = glm::mat4(1.0f);
		model_matrix = glm::translate(model_matrix, player_position);
		model_matrix = model_matrix * glm::toMat4(player_orientation);
		model_matrix = glm::rotate(model_matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
		model_matrix = glm::scale(model_matrix, player_size);

		set_texture(get_textures()[animation.mesh.mesh_id]);
		draw(animation.mesh, model_matrix, view_matrix, camera.proj);

	}
}