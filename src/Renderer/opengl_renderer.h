#ifndef OPENGL_RENDERER_HEADER
#define OPENGL_RENDERER_HEADER

#define GLEW_STATIC
#include <GLEW/glew.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

typedef struct 
{
	unsigned int ID;
	std::string vertex_source_path;
	std::string fragment_source_path;
}ShaderProgram;

typedef struct
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;

}Vertex;


typedef struct
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	unsigned int bone_ids[4];
	glm::vec4 bone_weights;

}AnimVertex;

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
	int vertex_count;
	int index_count;
	unsigned int* index_buffer;
	AnimVertex* vertex_buffer;
}RawAnimMesh;

typedef struct
{
	unsigned int mesh_id;
	unsigned int index_count;
	
	unsigned int mesh_vbo;
	unsigned int mesh_ebo;
	unsigned int mesh_vao;
}Mesh;

typedef struct
{
	glm::vec3 min;
	glm::vec3 max;
}BoundingBox;

typedef struct
{
	float* height_values;
	float grid_width;
	float grid_height;
	int terrain_width;
	int terrain_height;
	glm::vec3 scale;
}TerrainMap;

TerrainMap* get_terrain_map();

void save_bounding_boxes();

void setup_gl_renderer();
Mesh& get_cube_mesh();
Mesh& get_water_mesh();
int get_num_meshes();
Mesh* get_meshes();
int get_num_textures();
GLuint* get_textures();
BoundingBox* get_meshes_bounding_box();
unsigned int get_next_mesh_id();//Pointless since meshes are in an array already

void load_all_meshes();
void load_all_textures();

void upload_image(GLuint& texture_handle, GLuint texture_slot, void* image_buffer, unsigned int image_width, unsigned int image_height);
void loadShader(ShaderProgram& shaderprogram);
Mesh upload_raw_mesh(RawMesh& raw_mesh);
Mesh upload_raw_anim_mesh(RawAnimMesh& raw_mesh);

void use_shader(ShaderProgram& shader);
void start_next_frame();
void set_texture(GLuint texture);
void draw(Mesh m, glm::mat4& model_matrix, glm::mat4& view_matrix, glm::mat4& projection_matrix, GLuint primitives = GL_TRIANGLES);

#endif