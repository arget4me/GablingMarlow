#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "globals.h"
#include "world.h"


local_scope const float n = 0.001f;
local_scope const float f = 1000.0f;

Camera get_default_camera(float screen_width, float screen_height)
{
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), screen_width / screen_height, n, f);

	return{ 
		{0,  0,  4},	//position
		{0,  0, -1},	//dir
		{1,  0,  0},	//right
		{proj}, //Proj
		{glm::inverse(proj)}, //Inv_proj
		{0.0f}, //pitch
		{-90.0f}, //yaw
	};
}

void recalculate_projection_matrix(Camera& camera, float screen_width, float screen_height)
{
	camera.proj = glm::perspective(glm::radians(45.0f), screen_width / screen_height, n, f);
	camera.inv_proj = glm::inverse(camera.proj);
}

Ray get_ray(Camera &camera, float x_pos, float y_pos)
{

	glm::mat4 inv_view = get_inverse_view_matrix(camera);
	
	glm::vec4 ray_clip(x_pos, y_pos, -1.0f, 1.0f);
	glm::vec4 ray_eye = camera.inv_proj * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
	glm::vec3 ray_world = glm::normalize(glm::vec3(inv_view * ray_eye));

	return { 
		{camera.position}, //Origin
		{ray_world}, //Direction
	};
}

void update_camera_orientation(Camera& camera, float delta_yaw, float delta_pitch, glm::vec3* follow)
{
	if(follow == nullptr)
	{
		if ((show_debug_panel && mouse_keys[2]) || !show_debug_panel)//Click to move camera in editor state
		{
			camera.yaw += delta_yaw;
			camera.pitch += delta_pitch;

			if (camera.pitch > 89.0f)
				camera.pitch = 89.0f;
			else if (camera.pitch < -89.0f)
				camera.pitch = -89.0f;
		
		}

		camera.dir.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		camera.dir.y = sin(glm::radians(camera.pitch));
		camera.dir.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
		camera.dir = glm::normalize(camera.dir);

		camera.right = glm::cross(camera.dir, glm::vec3(0, 1, 0));
	}
	else
	{
		static const float distance_from_object = 4.0f;
		if (camera.pitch < -5.0f)camera.pitch = -5.0f;
		if (camera.pitch > 35.0f)camera.pitch = 35.0f;
		glm::vec3 object_position = (*follow) +glm::vec3(0, 1.4f, 0);
		camera.position.y = object_position.y + (sin(glm::radians(camera.pitch)) * distance_from_object);
		camera.position.x = object_position.x - (sin(glm::radians(180 - camera.yaw)) * distance_from_object);
		camera.position.z = object_position.z - (cos(glm::radians(180 - camera.yaw)) * distance_from_object);

		camera.dir = glm::normalize(object_position - camera.position);
		camera.right = glm::cross(camera.dir, glm::vec3(0, 1, 0));
	}
}

void update_camera(Camera &camera, glm::vec3* follow)
{
	if(follow == nullptr)
	{
		if ((keys[0] == true) || (keys[4] == true))// W, UP
		{
			camera.position += camera_movement_speed * camera.dir;
		}
		if ((keys[1] == true) || (keys[5] == true))// A, LEFT
		{
			camera.position -= camera_movement_speed * camera.right;
		}
		if ((keys[2] == true) || (keys[6] == true))// S, DOWN
		{
			camera.position -= camera_movement_speed * camera.dir;
		}
		if ((keys[3] == true) || (keys[7] == true))// D, RIGHT
		{
			camera.position += camera_movement_speed * camera.right;
		}
	}
	else
	{
		update_camera_orientation(camera, 0, 0, follow);
		float terrain_height = get_terrain_height(camera.position);
		if (camera.position.y < terrain_height + 1.0f)
		{
			camera.position.y = terrain_height + 1.0f;
			glm::vec3 object_position = (*follow) + glm::vec3(0, 1.4f, 0);
			camera.dir = glm::normalize(object_position - camera.position);
		}
	}
}