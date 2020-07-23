#ifndef RAW_MESH_IO_HEADER
#define RAW_MESH_IO_HEADER
#include <string>
#include "opengl_renderer.h"

bool save_raw_mesh(std::string filepath, RawMesh& raw_mesh);
RawMesh load_raw_mesh(char* buffer, int buffersize);


#endif