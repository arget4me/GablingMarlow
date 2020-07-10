#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "globals.h"


local_scope const float n = 0.001f;
local_scope const float f = 1000.0f;

Camera get_default_camera(float screen_width, float screen_height)
{
	return{ 
		{0,  0,  4},	//position
		{0,  0, -1},	//dir
		{1,  0,  0},	//right
		{glm::perspective(glm::radians(45.0f), screen_width / screen_height, n, f)}, //Proj
		{0.0f}, //pitch
		{-90.0f}, //yaw
	};
}

void recalculate_projection_matrix(Camera& camera, float screen_width, float screen_height)
{
	camera.proj = glm::perspective(glm::radians(45.0f), screen_width / screen_height, n, f);
}

void update_camera_orientation(Camera& camera, float delta_yaw, float delta_pitch)
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

void update_camera(Camera &camera)
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