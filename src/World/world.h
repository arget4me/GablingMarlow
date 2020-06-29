#ifndef WORLD_HEADER
#define WORLD_HEADER

#include <string>
#include "Renderer/opengl_renderer.h"







int get_num_world_objects();
glm::vec3* get_world_object_positions();
unsigned int* get_world_object_mesh_indices();

bool load_world_from_file(std::string world_filepath);
bool save_world_to_file(std::string world_filepath);

void update_world();
void render_world(ShaderProgram &shader);


#endif