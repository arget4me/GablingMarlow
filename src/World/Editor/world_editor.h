#ifndef WORLD_EDITOR_HEADER
#define WORLD_EDITOR_HEADER
#include <World/Camera.h>
#include <Renderer/opengl_renderer.h>

unsigned int get_selected();
void init_editor();

void handle_editor_controlls(Camera& camera);

void toggle_object_editor();

void render_editor_overlay(ShaderProgram& shader, Camera& camera);

void editor_select_object(int index);


#endif