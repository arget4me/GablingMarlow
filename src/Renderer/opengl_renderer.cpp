#include "opengl_renderer.h"
#include "config.h"
#include <Utils/readfile.h>
#include <Utils/writefile.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "obj_loader.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "globals.h"

#include <stb_image.h>

#include <IMGUI/imgui.h>

local_scope int num_world_meshes;
local_scope Mesh* world_meshes;
local_scope BoundingBox* world_meshes_bounding_box;
local_scope GLuint bound_program = 0;

local_scope Mesh cube_mesh;


Mesh& get_cube_mesh() { return cube_mesh; }
int get_num_meshes() { return num_world_meshes; }
Mesh* get_meshes() { return world_meshes; }


BoundingBox* get_meshes_bounding_box() { return world_meshes_bounding_box; }

void load_bounding_boxes()
{
	int filesize = 0;
	std::string file = "data/models/bounding_boxes";
	get_filesize(file, &filesize);

	if (filesize > 0)
	{
		if(num_world_meshes * sizeof(BoundingBox) <= filesize)
			if (read_buffer(file, world_meshes_bounding_box, num_world_meshes * sizeof(BoundingBox)) != -1)
			{

			}
	}


}

void save_bounding_boxes()
{
	std::string file = "data/models/bounding_boxes";
	if (write_buffer_overwrite(file, world_meshes_bounding_box, num_world_meshes * sizeof(BoundingBox)) != -1)
	{

	}
}

void load_all_meshes()
{
	RawMesh raw_mesh[5];

	raw_mesh[0] = load_obj_allocate_memory("data/models/dice_smooth.obj");
	raw_mesh[1] = load_obj_allocate_memory("data/models/test_model_2.obj");
	raw_mesh[2] = load_obj_allocate_memory("data/models/prototype_tree.obj");
	raw_mesh[3] = load_obj_allocate_memory("data/models/test_model.obj");
	raw_mesh[4] = load_obj_allocate_memory("data/models/prototype_island_2.obj");

	//raw_mesh[4] = load_obj_allocate_memory("data/models/prototype_island.obj");

	num_world_meshes = 5;
	world_meshes = new Mesh[num_world_meshes];
	world_meshes_bounding_box = new BoundingBox[num_world_meshes];
	for (int i = 0; i < num_world_meshes; i++)
	{
		world_meshes[i] = upload_raw_mesh(raw_mesh[i]);
		BoundingBox& box = world_meshes_bounding_box[i];


		for (int k = 0; k < raw_mesh[i].vertex_count; k++)
		{
			glm::vec3& pos = raw_mesh[i].vertex_buffer[k].position;

			if (pos.x < box.min.x)
				box.min.x = pos.x;

			if (pos.x > box.max.x)
				box.max.x = pos.x;

			if (pos.y < box.min.y)
				box.min.y = pos.y;

			if (pos.y > box.max.y)
				box.max.y = pos.y;

			if (pos.z < box.min.z)
				box.min.z = pos.z;

			if (pos.z > box.max.z)
				box.max.z = pos.z;

		}
	}
	load_bounding_boxes();


	for (int i = 0; i < num_world_meshes; i++)
	{
		delete[] raw_mesh[i].index_buffer;
		delete[] raw_mesh[i].vertex_buffer;
	}

	RawMesh raw_cube_mesh = load_obj_allocate_memory("data/models/cube.obj");

	cube_mesh = upload_raw_mesh(raw_cube_mesh);

	delete[] raw_cube_mesh.index_buffer;
	delete[] raw_cube_mesh.vertex_buffer;
}

void checkShaderCompileError(GLint shaderID)
{
	GLint isCompiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

		DEBUG_LOG("shader compilation failed:\n");
		DEBUG_LOG(errorLog << "\n");
		return;
	}
	else
		DEBUG_LOG("shader compilation success.\n");

	return;
}

void loadShader(ShaderProgram & shaderprogram)
{
	//Read vertex sourcefile
	int vertex_filesize = 0;
	get_filesize(shaderprogram.vertex_source_path, &vertex_filesize);
	if (vertex_filesize == -1)
		return;
	char* vertex_shader_src = new char[vertex_filesize + 1];
	vertex_shader_src[vertex_filesize] = '\0';
	read_buffer(shaderprogram.vertex_source_path, vertex_shader_src, vertex_filesize);

	//Load fragment sourcefile
	int fragment_filesize = 0;
	get_filesize(shaderprogram.fragment_source_path, &fragment_filesize);
	if (fragment_filesize == -1)
		return;
	char* fragment_shader_src = new char[fragment_filesize + 1];
	fragment_shader_src[fragment_filesize] = '\0';
	read_buffer(shaderprogram.fragment_source_path, fragment_shader_src, fragment_filesize);

	//Comiple and link shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader_src, NULL);
	glCompileShader(vs);
	checkShaderCompileError(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader_src, NULL);
	glCompileShader(fs);
	checkShaderCompileError(fs);

	shaderprogram.ID = glCreateProgram();
	glAttachShader(shaderprogram.ID, fs);
	glAttachShader(shaderprogram.ID, vs);
	glLinkProgram(shaderprogram.ID);

	//Cleanup
	glDeleteShader(vs);
	glDeleteShader(fs);
	delete[] vertex_shader_src;
	delete[] fragment_shader_src;
}


void setup_gl_renderer()
{
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
}

GLuint get_next_texture_slot()
{
	static GLuint next_texture_slot = 0;
	GLuint return_value = next_texture_slot;
	next_texture_slot = next_texture_slot + 1;

	return return_value;
}


void upload_image(GLuint &texture_handle, GLuint texture_slot, void* image_buffer, unsigned int image_width, unsigned int image_height)
{
	if (texture_slot >= 16)
	{
		ERROR_LOG("Too many texture slots, can't access slot " << texture_slot);
		return;
	}

	glGenTextures(1, &texture_handle);

	glActiveTexture(GL_TEXTURE0 + texture_slot);
	glBindTexture(GL_TEXTURE_2D, texture_handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
	glGenerateMipmap(GL_TEXTURE_2D);
}

void load_all_textures()
{
	static GLuint texture_handle[5];
	const char* texture_files[5] = {
		"data/textures/dice_smooth_texture.png",
		"data/textures/test_model_2_texture.png",
		"data/textures/prototype_tree_texture.png",
		"data/textures/test_model_texture.png",
		"data/textures/prototype_island_2_texture.png",
	};


	for(int i = 0; i < 5; i++)
	{
		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(texture_files[i], &width, &height, &nrChannels, 0);
		if (data)
		{

			upload_image(texture_handle[i], get_next_texture_slot(), data, width, height);
			//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			//glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			DEBUG_LOG("Failed to load texture\n");
		}
		stbi_image_free(data);
	}
	return;
}


unsigned int get_next_mesh_id()
{
	static unsigned int id = 0;
	return id++;
}

Mesh upload_raw_mesh(RawMesh& raw_mesh)
{
	Mesh m;
	m.mesh_id = raw_mesh.mesh_id;
	m.index_count = raw_mesh.index_count;

	glGenVertexArrays(1, &m.mesh_vao);
	glGenBuffers(1, &m.mesh_vbo);
	glGenBuffers(1, &m.mesh_ebo);
	
	glBindVertexArray(m.mesh_vao);

	//upload vertices
	glBindBuffer(GL_ARRAY_BUFFER, m.mesh_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * raw_mesh.vertex_count, raw_mesh.vertex_buffer, GL_STATIC_DRAW);

	//upload indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.mesh_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, raw_mesh.index_count * sizeof(unsigned int), raw_mesh.index_buffer, GL_STATIC_DRAW);

	//describe vertex layout
	//positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	
	//normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	//uv coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));


	glBindVertexArray(0);

	return m;
}

void use_shader(ShaderProgram& shader)
{
	//Do checks if already bound.
	if (bound_program != shader.ID)
	{
		glUseProgram(shader.ID);
		bound_program = shader.ID;
		
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_0"), 0);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_1"), 1);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_2"), 2);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_3"), 3);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_4"), 4);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_5"), 5);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_6"), 6);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_7"), 7);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_8"), 8);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_9"), 9);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_10"), 10);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_11"), 11);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_12"), 12);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_13"), 13);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_14"), 14);
		glUniform1i(glGetUniformLocation(shader.ID, "diffuse_texture_15"), 15);
	}
}

void draw(Mesh m, glm::mat4& model_matrix, glm::mat4& view_matrix, glm::mat4& projection_matrix, GLuint primitives)
{
	static GLuint bound_mesh = 0;

	glm::mat4 mvp_matrix = projection_matrix * view_matrix * model_matrix;

	glUniformMatrix4fv(glGetUniformLocation(bound_program, "mvp"), 1, GL_FALSE, glm::value_ptr(mvp_matrix));
	glUniformMatrix4fv(glGetUniformLocation(bound_program, "m"), 1, GL_FALSE, glm::value_ptr(model_matrix));
	
	if (bound_mesh != m.mesh_vao)
	{
		glBindVertexArray(m.mesh_vao);
		bound_mesh = m.mesh_vao;
		
		//Set active texture
	}
	glUniform1i(glGetUniformLocation(bound_program, "active_texture"), m.mesh_id);
	
	glDrawElements(primitives, m.index_count, GL_UNSIGNED_INT, 0);
}