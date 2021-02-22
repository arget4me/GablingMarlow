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
#include <Utils/structured_binary/structured_binary_io.h>
#include <Utils/structured_binary/structured_binary_wrapper.h>
#include "Utils/structured_binary/structured_binary_imgui_integration.h"
#include <Utils/value_compare.h>

local_scope bool edit_object_state = false;

local_scope int selected_object = 0;
local_scope float pulse_highlight = 0.5f;
local_scope bool pulse_highlight_state = true;
local_scope int selected_mesh = 0;
local_scope bool show_bounding_boxes = false;

unsigned int get_selected()
{
	return selected_object;
}

bool get_editor_state()
{
	return edit_object_state;
}


void handle_editor_controlls(Camera& camera, float deltaTime)
{
	update_camera(camera, deltaTime);
	pulse_float(pulse_highlight, pulse_highlight_state, 0.3f * deltaTime, 0.0f, 0.6f);
}

void editor_select_object(int index)
{
	selected_object = index;
}

void toggle_object_editor()
{
	edit_object_state = !edit_object_state;
}

void render_cubes(GLuint &color_shader_location, Camera &camera, glm::mat4 &view_matrix, glm::vec3 position, glm::vec3 size, glm::vec3 color_fill, glm::vec3 color_line)
{
	glm::mat4 model_matrix(1.0f);
	//fill min
	glUniform3fv(color_shader_location, 1, (float*)&color_fill);
	model_matrix = glm::translate(glm::mat4(1.0f), position);
	model_matrix = glm::scale(model_matrix, size);
	draw(get_cube_mesh(), model_matrix, view_matrix, camera.proj);


	//outline min
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform3fv(color_shader_location, 1, (float*)&color_line);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(0.0, -1.0);
	glLineWidth(2.0f);
	draw(get_cube_mesh(), model_matrix, view_matrix, camera.proj);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void render_bounding_boxes(ShaderProgram& shader, Camera& camera)
{
	use_shader(shader);
	static GLuint color_shader_location = glGetUniformLocation(shader.ID, "color");
	glUniform3f(color_shader_location, 1.0f, 1.0f, 0.0f);
	if (edit_object_state)
	{

		glm::mat4 view_matrix = get_view_matrix(camera);
		int num_meshes = get_num_meshes();
		unsigned int index = selected_mesh % num_meshes;
		glm::mat4 model_matrix = glm::mat4(1.0f);

		//bounding box
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		BoundingBox& box = get_meshes_bounding_box()[index];
		model_matrix = glm::translate(model_matrix, (box.max - box.min) / 2.0f + box.min);
		model_matrix = glm::scale(model_matrix, (box.max - box.min) / 2.0f);
		draw(get_cube_mesh(), model_matrix, view_matrix, camera.proj);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		glm::vec3 size(0.05f);
		glm::vec3 color_fill(0.5f, 0.0f, 0.0f);
		glm::vec3 color_line(1.0f, 0.0f, 0.0f);

		//min
		size = glm::vec3(0.05f);
		render_cubes(color_shader_location, camera, view_matrix, box.min, size, color_fill, color_line);
		//size = glm::vec3(0.1f, 0.01f, 0.01f);
		//render_cubes(color_shader_location, camera, view_matrix, box.min + glm::vec3(size.x, 0, 0), size, size*10.0f, size * 20.0f);
		//size = glm::vec3(0.01f, 0.1f, 0.01f);
		//render_cubes(color_shader_location, camera, view_matrix, box.min + glm::vec3(0, size.y, 0), size, size*10.0f, size * 20.0f);
		//size = glm::vec3(0.01f, 0.01f, 0.1f);
		//render_cubes(color_shader_location, camera, view_matrix, box.min + glm::vec3(0, 0, size.z), size, size*10.0f, size * 20.0f);
		
		//max
		size = glm::vec3(0.05f);
		render_cubes(color_shader_location, camera, view_matrix, box.max, size, color_fill, color_line);
		//size = glm::vec3(0.1f, 0.01f, 0.01f);
		//render_cubes(color_shader_location, camera, view_matrix, box.max + glm::vec3(size.x, 0, 0), size, size * 10.0f, size * 20.0f);
		//size = glm::vec3(0.01f, 0.1f, 0.01f);
		//render_cubes(color_shader_location, camera, view_matrix, box.max + glm::vec3(0, size.y, 0), size, size * 10.0f, size * 20.0f);
		//size = glm::vec3(0.01f, 0.01f, 0.1f);
		//render_cubes(color_shader_location, camera, view_matrix, box.max + glm::vec3(0, 0, size.z), size, size * 10.0f, size * 20.0f);

	}
	else
	{
		if (!show_bounding_boxes)return;
		int num_meshes = get_num_meshes();
		if (num_meshes > 0)
		{
			Mesh* meshes = get_meshes();
			int render_amount = get_num_world_objects_rendered();
			glm::mat4 view_matrix = get_view_matrix(camera);
			for (int i = 0; i < render_amount; i++)
			{
				unsigned int index = get_world_object_mesh_indices()[i];
				if (index < (unsigned int)num_meshes)
				{
					glm::mat4 model_matrix = glm::mat4(1.0f);
					model_matrix = glm::translate(model_matrix, get_world_object_positions()[i]);
					model_matrix = model_matrix * glm::toMat4(get_world_object_orientations()[i]);
					model_matrix = glm::scale(model_matrix, get_world_object_sizes()[i]);

					glDisable(GL_CULL_FACE);
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					BoundingBox& box = get_meshes_bounding_box()[index];
					model_matrix = glm::translate(model_matrix, (box.max - box.min) / 2.0f + box.min);
					model_matrix = glm::scale(model_matrix, (box.max - box.min) / 2.0f);
					draw(get_cube_mesh(), model_matrix, view_matrix, camera.proj);
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glEnable(GL_CULL_FACE);
				}
			}
		}

	}


	
}

void render_editor_overlay(ShaderProgram& shader, Camera& camera)
{
	use_shader(shader);
	static GLuint location_global_light = glGetUniformLocation(shader.ID, "global_light");
	glUniform4fv(location_global_light, 1, (float*)get_global_light_position()); 

	static GLuint location_sky_color = glGetUniformLocation(shader.ID, "sky_color");
	glUniform4fv(location_sky_color, 1, &color_dark[0]);

	static GLuint location_highlight_ratio = glGetUniformLocation(shader.ID, "highlight_ratio");

	if (edit_object_state)
	{
		glUniform1f(location_highlight_ratio, 0);
		glm::mat4 view_matrix = get_view_matrix(camera);
		int num_meshes = get_num_meshes();
		unsigned int index = selected_mesh % num_meshes;
		glm::mat4 model_matrix = glm::mat4(1.0f);
		draw(get_meshes()[index], model_matrix, view_matrix, camera.proj);

		glUniform1f(location_highlight_ratio, pulse_highlight);
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(0.0, -1.0);
		glLineWidth(2.0f);
		set_texture(get_textures()[index]);
		draw(get_meshes()[index], model_matrix, view_matrix, camera.proj);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glUniform1f(location_highlight_ratio, pulse_highlight);
		glm::mat4 view_matrix = get_view_matrix(camera);
		if (show_debug_panel)
		{
			unsigned int index = get_world_object_mesh_indices()[selected_object];
			if (index < (unsigned int) get_num_meshes())
			{

				glm::mat4 model_matrix = glm::mat4(1.0f);
				model_matrix = glm::translate(model_matrix, get_world_object_positions()[selected_object]);
				model_matrix = model_matrix * glm::toMat4(get_world_object_orientations()[selected_object]);
				model_matrix = glm::scale(model_matrix, get_world_object_sizes()[selected_object]);

				glDisable(GL_CULL_FACE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_POLYGON_OFFSET_LINE);
				glPolygonOffset(0.0, -1.0);
				glLineWidth(2.0f);
				set_texture(get_textures()[index]);
				draw(get_meshes()[index], model_matrix, view_matrix, camera.proj);
				glDisable(GL_POLYGON_OFFSET_LINE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_CULL_FACE); 
			}
		}
	}
}

#include <windows.h>
#include <Utils/logfile.h>

local_scope int worldfile_name_filter_callback(ImGuiTextEditCallbackData* data)
{
	auto& c = data->EventChar;
	switch (c)
	{
		case '/':
		case '<':
		case '>':
		case ':':
		case '|':
		case '?':
		case '*':
		case '\\':
		case '\"':
		{
			return 1;
		}break;


		default:
		{
			if (c <= (unsigned char)31)
			{
				return 1;
			}
		}break;
	}
	return 0;
}

local_scope char worldfile_textfield_input[128] = {};
static bool display_worldfile_textfield = false;

void render_world_imgui_layer(Camera& camera)
{
	render_imgui_structured_binary(global_structured_data);



	if (display_worldfile_textfield)
	{
		ImGui::Text("World file:"); ImGui::SameLine();
		{
			ImGui::PushItemWidth(200);
			int flags = ImGuiInputTextFlags_::ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter;
			ImGui::InputText("", worldfile_textfield_input, IM_ARRAYSIZE(worldfile_textfield_input), flags, &worldfile_name_filter_callback);
			ImGui::PopItemWidth();
		} ImGui::SameLine();
		if (strnlen_s(worldfile_textfield_input, IM_ARRAYSIZE(worldfile_textfield_input)) > 0)
		{
			//global_structured_data->value
			auto worldfile_list = STRUCTURED_IO::get_list_from_structure(global_structured_data->value);
			if (false == STRUCTURED_IO::check_list_contains_string(worldfile_list, worldfile_textfield_input, IM_ARRAYSIZE(worldfile_textfield_input)))
			{
				if (ImGui::Button("Create"))
				{
					DEBUG_LOG("@TODO: Create new empty worldfile \"" << worldfile_textfield_input << "\" Instead of just saving to that file\n");

					STRUCTURED_IO::add_value_to_end_list(worldfile_list, STRUCTURED_IO::add_text_null_terminated_value(worldfile_textfield_input));

					display_worldfile_textfield = !display_worldfile_textfield;

					int folder_string_length = VALUE_UTILS::null_terminated_char_string_length(WORLD_FOLDER_PATH, 128);
					int file_string_length = VALUE_UTILS::null_terminated_char_string_length(worldfile_textfield_input, 128);
					if (folder_string_length > 0 && folder_string_length <= 128)
					{
						if (file_string_length > 0 && file_string_length <= 128)
						{
							char new_file_path[128 * 2] = {};
							for (int i = 0; i < folder_string_length; i++)
							{
								new_file_path[i] = WORLD_FOLDER_PATH[i];
							}
							for (int i = 0; i < file_string_length; i++)
							{
								new_file_path[folder_string_length + i] = worldfile_textfield_input[i];
							}
							new_file_path[folder_string_length + file_string_length] = '\0';

							save_world_to_file(new_file_path);
						}
					}
				}
			}
			else
			{
				ImGui::Text("Name exist!");
			}
		}
		else
		{
			ImGui::Text("Add a name.");
		}

		if (ImGui::Button("Cancel"))
		{
			display_worldfile_textfield = !display_worldfile_textfield;
		}
	}
	else
	{
		if (ImGui::Button("Create new worldfile"))
		{
			display_worldfile_textfield = !display_worldfile_textfield;
		}
	}

	if (ImGui::Button("Toggle editor state"))
	{
		toggle_object_editor();
	}

	ImGui::ColorEdit3("Water-dark", &color_dark[0]);
	ImGui::ColorEdit3("Water-mid", &color_mid[0]);
	ImGui::ColorEdit3("Water-light", &color_light[0]);
	
	if (edit_object_state)
	{


		int num_meshes = get_num_meshes();
		unsigned int index = selected_mesh % num_meshes;

		ImGui::DragFloat3("Min", (float*)&get_meshes_bounding_box()[index].min, 0.1f);
		ImGui::DragFloat3("Max", (float*)&get_meshes_bounding_box()[index].max, 0.1f);
		if (ImGui::Button("Next mesh"))
		{

			selected_mesh++;
			if (selected_mesh >= get_num_meshes())
			{
				selected_mesh = 0;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Previous mesh"))
		{
			selected_mesh--;
			if (selected_mesh < 0)
			{
				selected_mesh = get_num_meshes() - 1;
			}
		}

		if (ImGui::Button("Save bounding boxes"))
		{
			save_bounding_boxes();
		}
	}
	else
	{
		{
			ImGui::SliderFloat3("Global light", (float*)get_global_light_position(), -50.0f, 50.0f);
			ImGui::SliderFloat3("Camera position", (float*)&camera.position, -50.0f, 50.0f);
		}

		ImGui::Separator();
		{
			ImGui::Text("Object modifiers");
			if (ImGui::Button("Toggle bounding boxes"))
			{
				show_bounding_boxes = !show_bounding_boxes;
			}

			int num_world_objects = get_num_world_objects();
			unsigned int& render_amount = get_num_world_objects_rendered();
			if (render_amount < (unsigned int)num_world_objects)
			{
				if (ImGui::Button("Add new object"))
				{
					render_amount++;
					if (render_amount >= (unsigned int)num_world_objects)
					{
						render_amount = num_world_objects;
					}
					unsigned int previously_selected_mesh = get_world_object_mesh_indices()[selected_object];
					glm::vec3 previously_selected_mesh_size = get_world_object_sizes()[selected_object];
					selected_object = render_amount - 1;
					get_world_object_positions()[selected_object] = camera.position + camera.dir * 3.0f;
					get_world_object_sizes()[selected_object] = previously_selected_mesh_size;
					get_world_object_orientations()[selected_object] = glm::quat(1, 0, 0, 0);
					get_world_object_mesh_indices()[selected_object] = previously_selected_mesh;
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
				for (unsigned int i = selected_object; i < render_amount - 1; i++)
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
				if ((unsigned int)selected_object >= render_amount)
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
			if (get_world_object_mesh_indices()[selected_object] == 4)
			{
				TerrainMap* terrain_map = get_terrain_map();
				terrain_map->scale = get_world_object_sizes()[selected_object];
			}
			ImGui::SliderFloat4("Quaternion (Orientation)", (float*)&get_world_object_orientations()[selected_object], -2.0f, 2.0f);
			get_world_object_orientations()[selected_object] = glm::normalize(get_world_object_orientations()[selected_object]);

			ImGui::InputInt("Model index: ", (int*)&get_world_object_mesh_indices()[selected_object]);
			get_world_object_mesh_indices()[selected_object] %= get_num_meshes();
		}

		ImGui::Separator();
		{
			if (ImGui::Button("Save world to testfile"))
			{



				save_world_to_file("data/world/testfile");
			}
		}

		ImGui::Separator();
		{//Post-processing

			if (ImGui::Button("Toggle post-processing"))
			{
				global_do_post_processing = !global_do_post_processing;
				glViewport(0, 0, global_width, global_height);
				glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
				glEnable(GL_DEPTH_TEST);
			}
			ImGui::SameLine();
			if (ImGui::Button("Next post-processing effect"))
			{
				global_render_style++;
				if (global_render_style > 5)global_render_style = 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Toggle outlines"))
			{
				global_render_outlines = !global_render_outlines;
			}
		}
	}
}





