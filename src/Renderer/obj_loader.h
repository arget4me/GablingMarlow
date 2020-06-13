#ifndef OBJ_LOADER_HEADER
#define OBJ_LOADER_HEADER
#include "config.h"

struct ObjInfo
{
	int num_faces = 0;
	int num_vertices = 0;
};

#ifdef TEST_LOADOBJ
void test_loadobj();
#endif // TEST_LOADOBJ




void loadobj(char* buffer, int buffersize);

void loadobj_info(char* buffer, int buffersize, struct ObjInfo& info);



#endif