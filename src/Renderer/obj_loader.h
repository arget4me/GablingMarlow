#ifndef OBJ_LOADER_HEADER
#define OBJ_LOADER_HEADER
#include "config.h"

#include <Renderer/opengl_renderer.h>

struct ObjInfo
{
	int num_faces = 0;
	int num_vertices = 0;
	int num_pos = 0;
	int num_textures = 0;
	int num_normals = 0;
};

#ifdef TEST_LOADOBJ
void test_loadobj();
#endif // TEST_LOADOBJ



/*
Declare memory outside!
Eg.
	float*  intermediate_data = new float[info.num_pos*3 + info.num_textures*2 + info.num_normals * 3];
	float* vertex_buffer = new float[info.num_vertices * sizeof(Vertex)];
	unsigned int* index_buffer = new unsigned int[info.num_faces * 3]; //Only supports triangle faces.
*/
bool loadobj(char* buffer, int buffersize, struct ObjInfo& info, float* intermediate_data, Vertex* vertex_buffer, unsigned int* index_buffer);

void loadobj_info(char* buffer, int buffersize, struct ObjInfo& info);



#endif