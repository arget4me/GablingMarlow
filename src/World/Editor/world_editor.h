#ifndef WORLD_EDITOR_HEADER
#define WORLD_EDITOR_HEADER
#include <World/Camera.h>
#include <Renderer/opengl_renderer.h>

unsigned int get_selected();

bool get_editor_state();

void handle_editor_controlls(Camera& camera, float deltaTime);

void toggle_object_editor();

void render_bounding_boxes(ShaderProgram& shader, Camera& camera);

void render_editor_overlay(ShaderProgram& shader, Camera& camera);

void editor_select_object(int index);

void render_world_imgui_layer(Camera& camera);


#endif