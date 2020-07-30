#ifndef DAE_LOADER_HEADER
#define DAE_LOADER_HEADER

#include "config.h"
#include <Renderer/opengl_renderer.h>
#include <string>
#include "animation_manager.h"

bool load_dae(std::string filepath, RawAnimMesh* raw_mesh, AnimatedMesh* animated);

#endif // !DAE_LOADER_HEADER
