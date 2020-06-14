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
	glm::vec2 uv;
	glm::vec4 normal;
	glm::vec3 color;

}Vertex;

typedef struct
{
	int vertex_count;
	int index_count;
	unsigned int* index_buffer;
	Vertex* vertex_buffer;
}Mesh;



void setup_gl_renderer();



#endif