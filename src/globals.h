#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "config.h"
#include <Renderer/animation_manager.h>

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

global_scope AnimatedMesh animation = {0};
global_scope glm::vec4 offsets(0);
global_scope glm::vec4 color_dark(0.0f, 0.0f, 0.5f, 1.0f);
global_scope glm::vec4 color_mid(0.35, 0.4, 0.65, 1);
global_scope glm::vec4 color_light(0.85, 0.9, 0.95, 1);

#else
global_scope AnimatedMesh animation;
global_scope int global_height;
global_scope int global_width;
global_scope bool keys[];
global_scope bool mouse_keys[];
global_scope bool show_debug_panel;
global_scope float camera_movement_speed;
global_scope glm::vec4 offsets;
global_scope glm::vec4 color_dark;
global_scope glm::vec4 color_mid;
global_scope glm::vec4 color_light;
#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
