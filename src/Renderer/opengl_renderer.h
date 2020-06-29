#ifndef OPENGL_RENDERER_HEADER
#define OPENGL_RENDERER_HEADER

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <string>

#include <glm/glm.hpp>

typedef struct 
{
	unsigned int ID;
	std::string vertex_source_path;
	std::string fragment_source_path;
}ShaderProgram;

typedef struct
{
	glm::vec4 position;
	glm::vec4 normal;
	glm::vec2 uv;
	glm::vec3 color;

}Vertex;

typedef struct
{
	unsigned int mesh_id;
	int vertex_count;
	int index_count;
	unsigned int* index_buffer;
	Vertex* vertex_buffer;
}RawMesh;

typedef struct
{
	unsigned int mesh_id;
	unsigned int index_count;
	
	unsigned int mesh_vbo;
	unsigned int mesh_ebo;
	unsigned int mesh_vao;
}Mesh;



int get_num_meshes();
Mesh* get_meshes();

void load_all_meshes();

void setup_gl_renderer();

void loadShader(ShaderProgram& shaderprogram);

unsigned int get_next_mesh_id();

Mesh upload_raw_mesh(RawMesh& raw_mesh);

void draw(Mesh m, ShaderProgram& shader, glm::vec3 model_origin = glm::vec3(0), float time = 0.0f, glm::vec3 size = glm::vec3(1.0f));


#endif