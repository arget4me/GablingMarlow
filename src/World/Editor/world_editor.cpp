#include "world_editor.h"
#include <Renderer/obj_loader.h>
#include <Renderer/opengl_renderer.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "config.h"
#include "globals.h"
#include <Utils/value_modifiers.h>
#include <imgui.h>
#include <World/world.h>


local_scope bool edit_object_state = false;
local_scope Mesh cube_mesh;

local_scope int selected_object = 0;
local_scope float pulse_highlight = 0.5f;
local_scope bool pulse_highlight_state = true;

unsigned int get_selected()
{
	return selected_object;
}

void init_editor()
{
	RawMesh raw_cube_mesh = load_obj_allocate_memory("data/models/cube.obj");

	cube_mesh = upload_raw_mesh(raw_cube_mesh);

	delete[] raw_cube_mesh.index_buffer;
	delete[] raw_cube_mesh.vertex_buffer;
}

void handle_editor_controlls(Camera& camera)
{
	pulse_float(pulse_highlight, pulse_highlight_state, 0.005f, 0.0f, 0.6f);
}

void editor_select_object(int index)
{
	selected_object = index;
}

void toggle_object_editor()
{

}

void render_editor_overlay(ShaderProgram& shader, Camera& camera)
{
	use_shader(shader);
	glm::mat4 view_matrix = get_view_matrix(camera);
	if (show_debug_panel)
	{
		glUniform1f(glGetUniformLocation(shader.ID, "highlight_ratio"), pulse_highlight);
		glUniform4fv(glGetUniformLocation(shader.ID, "global_light"), 1, (float*)get_global_light_position());

		unsigned int index = get_world_object_mesh_indices()[selected_object];
		if (index < get_num_meshes())
		{

			glm::mat4 model_matrix = glm::mat4(1.0f);
			model_matrix = glm::translate(model_matrix, get_world_object_positions()[selected_object]);
			model_matrix = model_matrix * glm::toMat4(get_world_object_orientations()[selected_object]);
			model_matrix = glm::scale(model_matrix, get_world_object_sizes()[selected_object]);


			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonOffset(0.0, -1.0);
			glLineWidth(2.0f);
			draw(get_meshes()[index], model_matrix, view_matrix, camera.proj);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

void render_world_imgui_layer(Camera& camera)
{
	ImGui::SliderFloat3("Global light", (float*)get_global_light_position(), -50.0f, 50.0f);
	ImGui::SliderFloat3("Camera position", (float*)&camera.position, -50.0f, 50.0f);
	ImGui::Separator();
	ImGui::Text("Object modifiers");

	int num_world_objects = get_num_world_objects();
	unsigned int& render_amount = get_num_world_objects_rendered();
	if (render_amount < num_world_objects)
	{
		if (ImGui::Button("Add new object"))
		{
			render_amount++;
			if (render_amount >= num_world_objects)
			{
				render_amount = num_world_objects;
			}
			selected_object = render_amount - 1;
			get_world_object_positions()[selected_object] = camera.position + camera.dir * 3.0f;
			get_world_object_sizes()[selected_object] = glm::vec3(1);
			get_world_object_orientations()[selected_object] = glm::quat(1, 0, 0, 0);
		}
	}
	else
	{
		if (ImGui::Button("[Memory is full, can't add more]"))
		{
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove selected object"))
	{
		for (int i = selected_object; i < render_amount - 1; i++)
		{
			get_world_object_positions()[i] = get_world_object_positions()[i + 1];
			get_world_object_sizes()[i] = get_world_object_sizes()[i + 1];
			get_world_object_orientations()[i] = get_world_object_orientations()[i + 1];
			get_world_object_mesh_indices()[i] = get_world_object_mesh_indices()[i + 1];
		}

		render_amount--;
		if (render_amount < 0)
		{
			render_amount = 0;
		}
		selected_object--;
		if (selected_object < 0)
		{
			selected_object = 0;
		}

	}

	if (ImGui::Button("Select next object"))
	{

		selected_object++;
		if (selected_object >= render_amount)
		{
			selected_object = 0;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Select previous object"))
	{
		selected_object--;
		if (selected_object < 0)
		{
			selected_object = render_amount - 1;
		}
	}
	ImGui::DragFloat3("Position", (float*)&get_world_object_positions()[selected_object], 0.1f);
	ImGui::DragFloat3("Size", (float*)&get_world_object_sizes()[selected_object], 0.1f, 0.0f, 100.0f);
	ImGui::SliderFloat4("Quaternion (Orientation)", (float*)&get_world_object_orientations()[selected_object], -2.0f, 2.0f);
	get_world_object_orientations()[selected_object] = glm::normalize(get_world_object_orientations()[selected_object]);

	ImGui::InputInt("Model index: ", (int*)&get_world_object_mesh_indices()[selected_object]);
	get_world_object_mesh_indices()[selected_object] %= get_num_meshes();

	ImGui::Separator();
	if (ImGui::Button("Save world to testfile"))
	{
		save_world_to_file("data/world/testfile");
	}
}





