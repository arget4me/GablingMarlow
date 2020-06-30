#include "obj_loader.h"
#include <Utils/readfile.h>
#include <string>

#include <glm/glm.hpp>

#include <Renderer/opengl_renderer.h>//@Note:To get Veretex, maybe move vetrex later

#include <map>
struct FaceIndexValue
{
	int vertex_pos;
	int texture_pos;
	int normal_pos;
};

inline bool operator<(const FaceIndexValue& lhs, const FaceIndexValue& rhs)
{
	bool pos = (lhs.vertex_pos < rhs.vertex_pos);
	bool tex = (lhs.vertex_pos == rhs.vertex_pos) && (lhs.texture_pos < rhs.texture_pos);
	bool norm = (lhs.vertex_pos == rhs.vertex_pos) && (lhs.texture_pos == rhs.texture_pos) && (lhs.normal_pos < rhs.normal_pos);


	return (pos) || (tex) || (norm);
}

std::map<FaceIndexValue, unsigned int> indexMap;

enum class TOKEN_TYPE{
	VERTEX,
	NORMAL,
	TEXTURE,
	FACE,
	NUM,
	FACEINDEX,
	OTHER
};

RawMesh load_obj_allocate_memory(std::string obj_filepath)
{
	int filesize;
	char* buffer;
	RawMesh m;
	m.mesh_id = 0;
	m.index_count = 0;
	m.index_buffer = nullptr;
	m.vertex_count = 0;
	m.vertex_buffer = nullptr;


	get_filesize(obj_filepath, &filesize);
	if (filesize != -1)
	{
		int padded_filesize = (filesize + 1);
		buffer = new char[padded_filesize];
		buffer[filesize] = '\0';
		if (read_buffer(obj_filepath, buffer, filesize) != -1)
		{
			struct ObjInfo info;
			loadobj_info(buffer, filesize, info);

			if (info.num_vertices > 0)
			{
				float* intermediate_data = new float[info.num_pos * 3 + info.num_textures * 2 + info.num_normals * 3];
				
				
				Vertex* vertex_buffer = new Vertex[info.num_vertices];
				unsigned int* index_buffer = new unsigned int[info.num_faces * 3];
				if (loadobj(buffer, filesize, info, intermediate_data, vertex_buffer, index_buffer))
				{
					m.mesh_id = get_next_mesh_id();
					m.index_count = info.num_faces * 3;
					m.vertex_count = info.num_vertices;
					m.index_buffer = index_buffer;
					m.vertex_buffer = vertex_buffer;
				}
				else
				{
					delete[] vertex_buffer;
					delete[] index_buffer;
				}
				delete[] intermediate_data;
			}
		}

		delete[] buffer;
	}

	return m;
}

#ifdef TEST_LOADOBJ
void test_loadobj()
{
	int filesize;
	char* buffer;
	std::string test_model_path = TEST_MODEL;
	get_filesize(test_model_path, &filesize);
	if (filesize != -1)
	{
		int padded_filesize = (filesize + 1);
		buffer = new char[padded_filesize];
		buffer[filesize] = '\0';
		if (read_buffer(test_model_path, buffer, filesize) != -1)
		{
			struct ObjInfo info;
			loadobj_info(buffer, filesize, info);

			if (info.num_vertices > 0)
			{
				float* intermediate_data = new float[info.num_pos * 3 + info.num_textures * 2 + info.num_normals * 3];
				Vertex* vertex_buffer = new Vertex[info.num_vertices];
				unsigned int* index_buffer = new unsigned int[info.num_faces * 3];
				if (loadobj(buffer, filesize, info, intermediate_data, vertex_buffer, index_buffer))
				{
					DEBUG_LOG("SUCCESSFULLY LOADED OBJ\n");
				}
				delete[] intermediate_data;
				delete[] vertex_buffer;
				delete[] index_buffer;
			}
		}
		
		delete[] buffer;
	}

}
#endif // TEST_LOADOBJ

void separate_tokens(char* buffer, int buffersize)
{
	bool remove_comment = false;
	for (int i = 0; i < buffersize; i++)
	{
		if (buffer[i] == '#')
		{
			remove_comment = true;
		}

		if (remove_comment)
		{
			if (buffer[i] == '\n' || buffer[i] == '\r')
			{
				remove_comment = false;
			}
			buffer[i] = '\0';
		}
		else
		{
			if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
			{
				buffer[i] = '\0';
			}
		}
	}
}

int get_token_size(char* buffer, int buffersize, int current_location)
{
	int token_size = 0;
	int i = current_location;
	while (i < buffersize)
	{
		if (buffer[i] == '\0')
			break;
		i++;
		token_size++;
	}

	return token_size;
}

void get_next_token(char* buffer, int buffersize, int& current_location)
{
	int token_size = get_token_size(buffer, buffersize, current_location);
	current_location += token_size;
	while ((current_location < buffersize) && (buffer[current_location] == '\0'))
	{
		current_location++;
	}
}

TOKEN_TYPE get_token_type(char* buffer, int buffersize, int current_location)
{
	char* token = buffer + current_location;
	int token_size = get_token_size(buffer, buffersize, current_location);

	TOKEN_TYPE type = TOKEN_TYPE::OTHER;

	if (token_size < 0)
	{
		return TOKEN_TYPE::OTHER;
	}

	if (token[0] == 'v')
	{
		if (token_size == 1)
		{
			return TOKEN_TYPE::VERTEX;
		}
		else if (token_size == 2)
		{
			if (token[1] == 'n')
			{
				return TOKEN_TYPE::NORMAL;
			}
			else if (token[1] == 't')
			{
				return TOKEN_TYPE::TEXTURE;
			}
		}
		else
		{
			return TOKEN_TYPE::OTHER;
		}
	}

	if (token[0] == 'f')
	{
		if (token_size == 1)
		{
			return TOKEN_TYPE::FACE;
		}
		else
		{
			return TOKEN_TYPE::OTHER;
		}
	}

	if (token[0] == '-' ||
		token[0] == '+' || 
		token[0] == '0' || 
		token[0] == '1' || 
		token[0] == '2' || 
		token[0] == '3' || 
		token[0] == '4' || 
		token[0] == '5' || 
		token[0] == '6' || 
		token[0] == '7' || 
		token[0] == '8' || 
		token[0] == '9')
	{
		if ((token[0] == '-' || token[0] == '+') && token_size == 1)
		{
			return TOKEN_TYPE::OTHER;
		}
		int num_dots = 0;
		int num_slashes = 0;

		for (int i = 1; i < token_size; i++)
		{
			if (token[i] == '/')
			{
				num_slashes++;
			}else if(token[i] == '.')
			{
				num_dots++;
			}
			else if (token[i] != '0' &&
				token[i] != '1' &&
				token[i] != '2' &&
				token[i] != '3' &&
				token[i] != '4' &&
				token[i] != '5' &&
				token[i] != '6' &&
				token[i] != '7' &&
				token[i] != '8' &&
				token[i] != '9')
			{
				return TOKEN_TYPE::OTHER;
			}
		}

		if ((num_dots == 1 || num_dots == 0) && num_slashes == 0 && token[token_size - 1] != '.')
		{
			return TOKEN_TYPE::NUM;
		}
		else if (num_dots == 0 && (num_slashes == 1 || num_slashes == 2))
		{
			if ((token[0] == '-' || token[0] == '+'))
			{
				return TOKEN_TYPE::OTHER;
			}

			int slash_index[2] = {0, 0};
			int k = 0;
			for (int i = 1; i < token_size; i++)
			{
				if (token[i] == '/')
				{
					slash_index[k++] == i;
					if(k >= 2)
						break;
				}
			}

			if (slash_index[0] == token_size - 1 || slash_index[1] == token_size - 1)
			{
				return TOKEN_TYPE::OTHER;
			}

			return TOKEN_TYPE::FACEINDEX;
		}
	}



	return TOKEN_TYPE::OTHER;
}

inline bool match(TOKEN_TYPE type, TOKEN_TYPE expected)
{
	return type == expected;
}

bool MatchVertex(char* buffer, int buffersize, int& current_location, float vertex_pos_data[3])
{

	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			ERROR_LOG("Can't parse vertex number " << i << ": " << token << "\n");
			return false;
		}
		else
		{
			vertex_pos_data[i] = std::stof(token);
		}
	}

	return true;
}

bool MatchNormal(char* buffer, int buffersize, int& current_location, float vertex_normal_data[3])
{
	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			ERROR_LOG("Can't parse vertex normal number " << i << ": " << token << "\n");
			return false;
		}
		else
		{
			vertex_normal_data[i] = std::stof(token);
		}
	}

	return true;
}

bool MatchTexture(char* buffer, int buffersize, int& current_location, float vertex_texture_data[2])
{
	for (int i = 0; i < 2; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			ERROR_LOG("Can't parse vertex texture number " << i << ": " << token << "\n");
			return false;
		}
		else
		{
			vertex_texture_data[i] = std::stof(token);
		}
	}

	return true;
}

bool MatchFace(char* buffer, int buffersize, int& current_location, unsigned int* index_buffer = nullptr, int* index_counter = nullptr)
{
	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		if (match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			int number = std::stoi(token);
			struct FaceIndexValue fiv = { number, 0, 0 };
			auto index = indexMap.find(fiv);
			if (index == indexMap.end())
			{
				if (index_counter == nullptr)
				{
					indexMap.insert(std::make_pair(fiv, 0));
				}
				else
				{
					if (index_buffer == nullptr)
					{
						return false;
					}
					(*index_counter)++;
					indexMap.insert(std::make_pair(fiv, *index_counter));
					index_buffer[i] = *index_counter;
				}
			}
			else
			{
				if (index_buffer != nullptr)
					index_buffer[i] = index->second;
			}
		}
		else if (match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::FACEINDEX))
		{
			int token_size = get_token_size(buffer, buffersize, current_location);

			int slash_index[2] = { 0, 0 };
			int k = 0;
			for (int t = 1; t < token_size; t++)
			{
				if (token[t] == '/')
				{
					token[t] = '\0';
					slash_index[k++] = t;
					if (k >= 2)
						break;
				}
			}

			int number_0 = std::stoi(token);
			int number_1 = 0;
			int number_2 = 0;
			if (slash_index[0] + 1 != slash_index[1])
			{
				char* mid_token = token + slash_index[0] + 1;
				number_1 = std::stoi(mid_token);
			}

			if (k == 2)
			{
				char* last_token = token + slash_index[1] + 1;
				number_2 = std::stoi(last_token);
			}

			if(slash_index[0] != 0)
				token[slash_index[0]] = '/';

			if (slash_index[1] != 0)
				token[slash_index[1]] = '/';

			struct FaceIndexValue fiv = {number_0, number_1, number_2};
			auto index = indexMap.find(fiv);
			if (index == indexMap.end())
			{
				if (index_counter == nullptr)
				{
					indexMap.insert(std::make_pair(fiv, 0));
				}
				else
				{
					if (index_buffer == nullptr)
					{
						return false;
					}
					(*index_counter)++;
					indexMap.insert(std::make_pair(fiv, *index_counter));
					index_buffer[i] = *index_counter;
				}
			}
			else
			{
				if(index_buffer != nullptr)
					index_buffer[i] = index->second;
			}
		}
		else
		{
			ERROR_LOG("Can't parse face index number " << i <<": " << token << "\n");
			return false;
		}
	}

	return true;
}



void loadobj_info(char* buffer, int buffersize, struct ObjInfo& info)
{
	indexMap.clear();
	separate_tokens(buffer, buffersize);
	int current_location = 0;
	info.num_faces = 0; //3 vertices per face
	info.num_pos = 0;
	info.num_normals = 0;
	info.num_textures = 0;
	info.num_vertices = 0; //One vertex with pos, normal, uv
	while (current_location < buffersize)
	{
		//Process current token
		TOKEN_TYPE type = get_token_type(buffer, buffersize, current_location);
		
		
		if (type == TOKEN_TYPE::VERTEX)
		{
			info.num_pos++;
		}
		else if (type == TOKEN_TYPE::TEXTURE)
		{
			info.num_textures++;
		}
		else if (type == TOKEN_TYPE::NORMAL)
		{
			info.num_normals++;
		}
		else if (type == TOKEN_TYPE::FACE)
		{
			if (MatchFace(buffer, buffersize, current_location))
			{
				info.num_faces++;
			}
		}
		else if (type == TOKEN_TYPE::NUM)
		{
		}
		else if (type == TOKEN_TYPE::FACEINDEX)
		{
		}
		else if (type == TOKEN_TYPE::OTHER)
		{
		}
		


		get_next_token(buffer, buffersize, current_location);

	}
	info.num_vertices = indexMap.size();
}


/*
Declare memory outside!
Eg.
	float*  intermediate_data = new float[info.num_pos*3 + info.num_textures*2 + info.num_normals * 3];
	float* vertex_buffer = new float[info.num_vertices * sizeof(Vertex)];
	unsigned int* index_buffer = new unsigned int[info.num_faces * 3]; //Only supports triangle faces.
*/
bool loadobj(char* buffer, int buffersize, struct ObjInfo& info, float* intermediate_data, Vertex* vertex_buffer, unsigned int* index_buffer)
{
	indexMap.clear();
	separate_tokens(buffer, buffersize);
	int current_location = 0;
	int pos_index = 0;
	int textures_index = 0;
	int normals_index = 0;
	int face_index = 0;
	int vertex_index = -1;
	while (current_location < buffersize)
	{
		//Process current token
		TOKEN_TYPE type = get_token_type(buffer, buffersize, current_location);
		
		
		if (type == TOKEN_TYPE::VERTEX)
		{
			if (pos_index < info.num_pos && MatchVertex(buffer, buffersize, current_location, intermediate_data + pos_index * 3))
			{
				pos_index++;
			}
			else
			{
				return false;
			}
		}
		else if (type == TOKEN_TYPE::TEXTURE)
		{
			if (textures_index < info.num_textures && MatchTexture(buffer, buffersize, current_location, intermediate_data + info.num_pos * 3 + textures_index * 2))
			{
				textures_index++;
			}
			else
			{
				return false;
			}
		}
		else if (type == TOKEN_TYPE::NORMAL)
		{
			if (normals_index < info.num_normals && MatchNormal(buffer, buffersize, current_location, intermediate_data + info.num_pos * 3 + info.num_textures * 2 + normals_index * 3))
			{
				normals_index++;
			}
			else
			{
				return false;
			}
		}
		else if (type == TOKEN_TYPE::FACE)
		{
			if (face_index < info.num_faces && MatchFace(buffer, buffersize, current_location, index_buffer + face_index * 3, &vertex_index))
			{
				face_index++;
			}
			else
			{
				return false;
			}
		}
		else if (type == TOKEN_TYPE::NUM)
		{
		}
		else if (type == TOKEN_TYPE::FACEINDEX)
		{
		}
		else if (type == TOKEN_TYPE::OTHER)
		{
		}



		get_next_token(buffer, buffersize, current_location);

	}
	info.num_vertices = indexMap.size();
	for (auto const& x : indexMap)
	{
		int p = x.first.vertex_pos - 1;
		int t = x.first.texture_pos - 1;
		int n = x.first.normal_pos -1;

		Vertex& v = vertex_buffer[x.second];
		v.position = glm::vec4(*(glm::vec3*)(intermediate_data+ p * 3), 1.0f);
		if (t >= 0 && t < info.num_textures)
			v.uv = *(glm::vec2*)(intermediate_data + info.num_pos * 3 + t * 2);
		else
			v.uv = glm::vec2(0.0f);

		if (n >= 0 && n < info.num_normals)
			v.normal = glm::vec4(*(glm::vec3*)(intermediate_data + info.num_pos * 3 + info.num_textures * 2 + n * 3), 0.0f);
		else
			v.normal = glm::vec4(0.0f);
		
		v.color = glm::vec3(1.0f);
	}
#ifdef TEST_LOADOBJ
	DEBUG_LOG("Num faces: " << info.num_faces << "\n");
	DEBUG_LOG("Num pos: " << info.num_pos << "\n");
	DEBUG_LOG("Num normals: " << info.num_normals << "\n");
	DEBUG_LOG("Num textures: " << info.num_textures << "\n");
	DEBUG_LOG("Num vertices: " << info.num_vertices << "\n");
#endif

	return true;
}
