#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "config.h"
#include <Renderer/opengl_renderer.h>

//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS

global_scope int global_height = 720;
global_scope int global_width = 1280;

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
global_scope bool mouse_keys[3] = { //Left, Middle, Right
	false,
	false,
	false
};

global_scope bool show_debug_panel = true;
global_scope float camera_movement_speed = 0.3f;

#else
global_scope Mesh dae_global_mesh;
global_scope int global_height;
global_scope int global_width;
global_scope bool keys[];
global_scope bool mouse_keys[];
global_scope bool show_debug_panel;
global_scope float camera_movement_speed;
#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
