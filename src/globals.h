#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "config.h"

//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS
global_scope int global_height = 720;
global_scope int global_width = 1280;
global_scope glm::vec3 camera_position(0, 0, 4);
global_scope glm::vec3 camera_facing(0, 0, -1);
global_scope glm::vec3 camera_up(0, 1, 0);
global_scope glm::vec3 camera_right(1, 0, 0);
global_scope bool keys[8] = {//W A S D | up left down right
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
};

#else
global_scope int global_height;
global_scope int global_width;
global_scope bool keys[];

global_scope glm::vec3 camera_position;
global_scope glm::vec3 camera_facing;
global_scope glm::vec3 camera_up;
global_scope glm::vec3 camera_right;
#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
