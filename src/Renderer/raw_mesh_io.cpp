#include "raw_mesh_io.h"
#include <Utils/writefile.h>



bool save_raw_mesh(std::string filepath, RawMesh& raw_mesh)
{
	int sizes[2] = { raw_mesh.index_count, raw_mesh.vertex_count};


	if (write_buffer_overwrite(filepath, sizes, sizeof(sizes)) == 0)
	{
		if (write_buffer_append(filepath, raw_mesh.index_buffer, sizeof(unsigned int) * sizes[0]) == 0)
		{
			if (write_buffer_append(filepath, raw_mesh.vertex_buffer, sizeof(Vertex) * sizes[1]) == 0)
			{
				return true;
			}
		}

	}

	return false;
}


RawMesh load_raw_mesh(char* buffer, int buffersize)
{
	RawMesh loaded_raw_mesh;
	loaded_raw_mesh.index_buffer = nullptr;
	loaded_raw_mesh.vertex_buffer = nullptr;
	if (buffersize > sizeof(int) * 2)
	{
		loaded_raw_mesh.index_count = *(int*)(buffer + 0);
		loaded_raw_mesh.vertex_count = *(int*)(buffer + 4);

		if (buffersize == sizeof(int) * 2
			+ loaded_raw_mesh.index_count * sizeof(unsigned int)
			+ loaded_raw_mesh.vertex_count * sizeof(Vertex))
		{
			loaded_raw_mesh.index_buffer = new unsigned int[loaded_raw_mesh.index_count];
			loaded_raw_mesh.vertex_buffer = new Vertex[loaded_raw_mesh.vertex_count];

			unsigned int* copy_index_buffer = (unsigned int*)(buffer + sizeof(int) * 2);
			for (int i = 0; i < loaded_raw_mesh.index_count; i++)
			{
				loaded_raw_mesh.index_buffer[i] = copy_index_buffer[i];
			}

			Vertex* copy_vertex_buffer = (Vertex*)(buffer + sizeof(int) * 2 + loaded_raw_mesh.index_count * sizeof(unsigned int));
			for (int i = 0; i < loaded_raw_mesh.vertex_count; i++)
			{
				loaded_raw_mesh.vertex_buffer[i] = copy_vertex_buffer[i];
			}
		}
	}
	loaded_raw_mesh.mesh_id = get_next_mesh_id();
	return loaded_raw_mesh;
}