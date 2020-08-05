#ifndef WORLD_HEADER
#define WORLD_HEADER

#include <string>
#include "Renderer/opengl_renderer.h"
#include "Camera.h"




bool ray_intersect_object_obb(Ray &ray, int& index_out);

int get_num_world_objects();
unsigned int& get_num_world_objects_rendered();
unsigned int* get_world_object_mesh_indices();
glm::vec3* get_world_object_positions();
glm::vec3* get_world_object_sizes();
glm::quat* get_world_object_orientations();
glm::vec3* get_global_light_position();

bool load_world_from_file(std::string world_filepath);
bool save_world_to_file(std::string world_filepath);

void update_world(Camera &camera);
void render_world(ShaderProgram &shader, Camera &camera);
void render_world_water(ShaderProgram &shader, Camera &camera);
void render_world_animations(ShaderProgram& shader, Camera& camera);


#endif