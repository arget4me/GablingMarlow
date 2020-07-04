#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER




//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS
extern int global_height = 720;
extern int global_width = 1280;
extern glm::vec3 camera_position(0, 0, 4);
extern glm::vec3 camera_facing(0, 0, -1);
extern glm::vec3 camera_up(0, 1, 0);
extern glm::vec3 camera_right(1, 0, 0);
bool keys[8] = {//W A S D | up left down right
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
extern int global_height;
extern int global_width;
bool keys[];

extern glm::vec3 camera_position;
extern glm::vec3 camera_facing;
extern glm::vec3 camera_up;
extern glm::vec3 camera_right;
#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
