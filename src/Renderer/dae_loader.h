#ifndef DAE_LOADER_HEADER
#define DAE_LOADER_HEADER

#include "config.h"
#include <Renderer/opengl_renderer.h>
#include <string>

RawMesh load_dae(std::string filepath);

#endif // !DAE_LOADER_HEADER
