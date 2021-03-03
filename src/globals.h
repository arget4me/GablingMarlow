#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "config.h"
#include <Renderer/animation_manager.h>
#include <Utils/structured_binary/structured_binary_io.h>

#define WORLD_FLODER_PATH "data/world/"
#define WORLD_FOLDER_PATH_ARRAY_SIZE sizeof((WORLD_FLODER_PATH))
#define WORLD_FOLDER_PATH_NUM_CHARCTERS (WORLD_FOLDER_PATH_ARRAY_SIZE - 1)

#define MESH_FLODER_PATH "data/models/raw_mesh/"
#define MESH_FOLDER_PATH_ARRAY_SIZE sizeof((MESH_FLODER_PATH))
#define MESH_FOLDER_PATH_NUM_CHARCTERS (MESH_FOLDER_PATH_ARRAY_SIZE - 1)

#define DEFAULT_STARTUP_FILE "testfile"
#define BACKUP_WORLD_FILE_NAME "backup_file"
#define STARTUP_WORLD_FILE_METAFILE "data/meta/startup_meta_file"
#define STARTUP_MESH_METAFILE "data/meta/mesh_meta_file"

//#define GLOBALS_DEFINITIONS
#ifdef GLOBALS_DEFINITIONS
global_scope const char global_world_folder_path[WORLD_FOLDER_PATH_ARRAY_SIZE] = WORLD_FLODER_PATH;
global_scope char global_world_file_path[WORLD_FOLDER_PATH_ARRAY_SIZE + 128] = WORLD_FLODER_PATH DEFAULT_STARTUP_FILE;
global_scope char global_world_backup_file_path[WORLD_FOLDER_PATH_ARRAY_SIZE + 128] = WORLD_FLODER_PATH BACKUP_WORLD_FILE_NAME;

global_scope const char global_mesh_folder_path[MESH_FOLDER_PATH_ARRAY_SIZE] = MESH_FLODER_PATH;
global_scope char global_mesh_file_path[MESH_FOLDER_PATH_ARRAY_SIZE + 128] = MESH_FLODER_PATH "";

global_scope STRUCTURED_IO::StructuredData* global_worldfile_names_meta_data = nullptr;
global_scope STRUCTURED_IO::StructuredData* global_worldfile_startup_meta_data = nullptr;
global_scope STRUCTURED_IO::StructuredData* mesh_meta_data = nullptr;
#if RECORDNING_MODE
global_scope int global_height = 1080;
global_scope int global_width = 1920;
#else
global_scope int global_height = 720;
global_scope int global_width = 1280;
#endif
global_scope float game_time = 0.0f;

//@Todo: Are these are post-processing stuff? bring the into their own struct.
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

global_scope const char global_world_folder_path[WORLD_FOLDER_PATH_ARRAY_SIZE];
global_scope char global_world_file_path[WORLD_FOLDER_PATH_ARRAY_SIZE + 128];
global_scope char global_world_backup_file_path[WORLD_FOLDER_PATH_ARRAY_SIZE + 128];

global_scope const char global_mesh_folder_path[MESH_FOLDER_PATH_ARRAY_SIZE];
global_scope char global_mesh_file_path[MESH_FOLDER_PATH_ARRAY_SIZE + 128];

global_scope STRUCTURED_IO::StructuredData* global_worldfile_names_meta_data;
global_scope STRUCTURED_IO::StructuredData* global_worldfile_startup_meta_data;
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
