#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "config.h"
#include <Renderer/animation_manager.h>

//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS

global_scope int global_height = 720;
global_scope int global_width = 1280;
global_scope float game_time = 0.0f;
global_scope GLuint rbo = 0;
global_scope GLuint texColorBuffer = 0;
global_scope GLuint framebuffer = 0;
global_scope GLuint texColorBuffer_post = 0;
global_scope GLuint framebuffer_post = 0;

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
global_scope float camera_movement_speed = 18; // units/s

global_scope AnimatedMesh animation = {0};
global_scope glm::vec4 offsets(0);
global_scope glm::vec4 color_dark(30 / 255.0f, 100 / 255.0f, 255 / 255.0f, 1.0f);
global_scope glm::vec4 color_mid(40 / 255.0f, 130 / 255.0f, 255 / 255.0f, 1.0f);
global_scope glm::vec4 color_light(220 / 255.0f, 230 / 255.0f, 240 / 255.0f, 1.0f);

global_scope bool global_do_post_processing = false;
global_scope int global_render_style = 1;
global_scope bool global_render_outlines = false;

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
global_scope float game_time;
global_scope GLuint rbo;
global_scope GLuint texColorBuffer;
global_scope GLuint framebuffer;
global_scope GLuint texColorBuffer_post;
global_scope GLuint framebuffer_post;
global_scope bool global_do_post_processing;
global_scope int global_render_style;
global_scope bool global_render_outlines;

#endif // GLOBALS_DEFINITIONS



#endif // !GLOBALS_HEADER
