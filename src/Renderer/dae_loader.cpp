
#include "dae_loader.h"
#include <Renderer/opengl_renderer.h>
#include <Utils/logfile.h>
#include <Utils/readfile.h>

#include <string>
#include <map>
#include <vector>
#include "globals.h"

local_scope void separate_tokens(char* buffer, int buffersize)
{
	bool remove_comment = false;
	bool check_if_comment_token = false;
	for (int i = 0; i < buffersize; i++)
	{
		if (check_if_comment_token)
		{
			if (remove_comment == false)
			{
				if (buffer[i] == '?')
				{
					buffer[i - 1] = '\0';
					remove_comment = true;
				}
			}
			else
			{
				if (buffer[i] == '>')
				{
					buffer[i] = '\0';
					remove_comment = false;
				}
			}
			check_if_comment_token = false;
		}

		if (remove_comment)
		{
			if (buffer[i] == '?')
			{
				check_if_comment_token = true;
			}

			buffer[i] = '\0';
		}
		else
		{
			if (buffer[i] == '<')
			{
				check_if_comment_token = true;
			}
			if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
			{
				buffer[i] = '\0';
			}
		}
	}
}



local_scope int get_token_size(char* buffer, int buffersize, int current_location)
{
	int token_size = 0;
	int i = current_location;
	bool check_ending_token = false;
	bool check_oneline_ending_token = false;
	while (i < buffersize)
	{
		if (buffer[i] == '\0')
			break;
	
		if (check_ending_token)
		{
			//check if token is '<' or '</'
			if (buffer[i] == '/')
			{
				token_size++;
				break;
			}
			else
			{
				break;
			}
		}

		if (check_oneline_ending_token)
		{
			//check if token is '/>' just a '/' in string 
			if (buffer[i] == '>')
			{
				if (token_size != 1)
					token_size--; //If size is larger than 1 it means there were characters before the '/' and should be handled before the end token
				else
					token_size++; //No character before the '/' Therefor it is a '/>' token
				break;
			}
			check_oneline_ending_token = false;
		}

		if (buffer[i] == '/')
		{
			check_oneline_ending_token = true;
		}

		if (buffer[i] == '>' || buffer[i] == '=')
		{
			//if token_size isn't 0, then the current token shouldn't include '>'
			if (token_size != 0)
				break;

			token_size++;
			break;
		}
		if (buffer[i] == '<')
		{
			//if token_size isn't 0, then the current token shouldn't include '<'
			if (token_size != 0)
				break;

			//enable check for token being '<' or '</'
			check_ending_token = true;
		}
		i++;
		token_size++;
	}

	return token_size;
}

local_scope void get_next_token(char* buffer, int buffersize, int& current_location)
{
	int token_size = get_token_size(buffer, buffersize, current_location);
	current_location += token_size;
	while ((current_location < buffersize) && (buffer[current_location] == '\0'))
	{
		current_location++;//Remove spacing between tokens
	}
}

typedef struct
{
	int length;
	char* data;
}TOKEN;

//the caller must delete the memory
local_scope TOKEN get_token(char* buffer, int buffersize, int current_location)
{
	int token_size = get_token_size(buffer, buffersize, current_location);
	TOKEN token;
	token.data = new char[token_size + 1];
	for (int i = 0; i < token_size; i++)
	{
		token.data[i] = buffer[current_location + i];
	}
	token.data[token_size] = '\0';
	token.length = token_size;

	return token;
}

local_scope enum class TOKEN_TYPE {
	NEW_BLOCK,
	NEW_CLOSE_BLOCK,
	END_BLOCK,
	EQUALS,
	STRING_VALUE,
	NAME,
	NUM,
	OTHER
};

local_scope TOKEN_TYPE get_token_type(TOKEN token)
{
	if (token.length == 1)
	{
		if (token.data[0] == '<')
		{
			return TOKEN_TYPE::NEW_BLOCK;
		}
		else if (token.data[0] == '>')
		{
			return TOKEN_TYPE::END_BLOCK;
		}
		else if (token.data[0] == '=')
		{
			return TOKEN_TYPE::EQUALS;
		}
	}
	else 
	{
		if (token.length == 2)
		{
			if (token.data[0] == '<' && token.data[1] == '/')
			{
				return TOKEN_TYPE::NEW_CLOSE_BLOCK;
			}
			if (token.data[0] == '/' && token.data[1] == '>')
			{
				return TOKEN_TYPE::END_BLOCK;
			}
		}

		if (token.data[0] == '"' && token.data[token.length-1] == '"')
		{
			return TOKEN_TYPE::STRING_VALUE;
		}
	}
	
	if (token.data[0] == '0' ||
		token.data[0] == '1' ||
		token.data[0] == '2' ||
		token.data[0] == '3' ||
		token.data[0] == '4' ||
		token.data[0] == '5' ||
		token.data[0] == '6' ||
		token.data[0] == '7' ||
		token.data[0] == '8' ||
		token.data[0] == '9' ||
		token.data[0] == '-' ||
		token.data[0] == '+')
	{
		bool is_number = true;
		int num_dots = 0;
		int num_exponents = 0;
		for (int i = 1; i < token.length; i++)
		{
			if (token.data[i] == '0' ||
				token.data[i] == '1' ||
				token.data[i] == '2' ||
				token.data[i] == '3' ||
				token.data[i] == '4' ||
				token.data[i] == '5' ||
				token.data[i] == '6' ||
				token.data[i] == '7' ||
				token.data[i] == '8' ||
				token.data[i] == '9' ||
				token.data[i] == '.' ||
				token.data[i] == 'e' ||
				token.data[i] == '-' ||
				token.data[i] == '+')
			{
				if (token.data[i] == '.')
				{
					num_exponents = num_exponents + 1;
					if (num_exponents > 1)
					{
						is_number = false;
						break;
					}
				}

				if (token.data[i] == '-' || token.data[i] == '+')
				{
					if (token.data[i - 1] != 'e')
					{
						is_number = false;
						break;
					}
				}
				if (token.data[i] == 'e')
				{
					num_dots = num_dots + 1;
					if (num_dots > 1)
					{
						is_number = false;
						break;
					}
				}
			}
			else
			{
				is_number = false;
				break;
			}
		}
		if (is_number)
		{
			if(token.data[0] != '\0')
				return TOKEN_TYPE::NUM;
		}

	}
	else
	{
		return TOKEN_TYPE::NAME;
	}

	return TOKEN_TYPE::OTHER;
}

local_scope void print_token_type(TOKEN_TYPE type)
{
	
	switch (type)
	{
	case TOKEN_TYPE::NEW_BLOCK:
	{
		DEBUG_LOG("NEW_BLOCK:\t");
	}break;

	case TOKEN_TYPE::NEW_CLOSE_BLOCK:
	{
		DEBUG_LOG("NEW_CLOSE_BLOCK:\t");
	}break;

	case TOKEN_TYPE::END_BLOCK:
	{
		DEBUG_LOG("END_BLOCK:\t");
	}break;

	case TOKEN_TYPE::EQUALS:
	{
		DEBUG_LOG("EQUALS:\t\t");
	}break;

	case TOKEN_TYPE::STRING_VALUE:
	{
		DEBUG_LOG("STRING_VALUE:\t");
	}break;

	case TOKEN_TYPE::NAME:
	{
		DEBUG_LOG("NAME:\t\t");
	}break;

	case TOKEN_TYPE::NUM:
	{
		DEBUG_LOG("NUM:\t\t");
	}break;

	case TOKEN_TYPE::OTHER:
	{
		DEBUG_LOG("OTHER:\t\t");
	}break;

	}
}



typedef struct
{
	std::string name;
	std::string value;
}FIELD;

typedef struct block
{
	bool is_value = false;
	float value;

	std::string name;
	std::vector<FIELD> fields;
	std::vector<block> values;
}BLOCK;

local_scope inline void next_token(char* buffer, int buffersize, int& current_location, TOKEN &token, TOKEN_TYPE &token_type)
{
	if (token.data != nullptr)
	{
		delete[] token.data;
		token.data = nullptr;
	}
	get_next_token(buffer, buffersize, current_location);
	token = get_token(buffer, buffersize, current_location);
	token_type = get_token_type(token);
}

/*
local_scope void print_tabs(int tab_depth)
{
	for (int i = 0; i < tab_depth; i++)
		DEBUG_LOG(" ");
}
*/

local_scope FIELD parse_field(char* buffer, int buffersize, int& current_location, TOKEN& token, TOKEN_TYPE& token_type, int tab_depth)
{

	FIELD field;
	if (token_type == TOKEN_TYPE::NAME)
	{
		field.name = std::string(token.data);

		next_token(buffer, buffersize, current_location, token, token_type);
		if (token_type == TOKEN_TYPE::EQUALS)
		{
			next_token(buffer, buffersize, current_location, token, token_type);

			if (token_type == TOKEN_TYPE::STRING_VALUE)
			{
				token.data[token.length - 1] = '\0';
				field.value = std::string(token.data + 1);

				//DEBUG_LOG(" " <<  field.name << "=" << field.value);
				return field;
			}
		}
	}

	ERROR_LOG("ERROR PARSING FIELD\n");

	return field;
}



local_scope BLOCK parse_block(char* buffer, int buffersize, int& current_location, TOKEN& token, TOKEN_TYPE& token_type, int tab_depth)
{
	BLOCK block;
	
	if (token_type == TOKEN_TYPE::NUM)
	{
		block.is_value = true;
		block.value = std::stof(token.data);
		//DEBUG_LOG(block.value << " ");
		return block;
	}
	else if (token_type == TOKEN_TYPE::NAME || token_type == TOKEN_TYPE::OTHER)
	{
		block.is_value = true;
		block.name = std::string(token.data);

		//DEBUG_LOG(block.name << " ");
		return block;
	}
	else if(token_type == TOKEN_TYPE::NEW_BLOCK)
	{
		next_token(buffer, buffersize, current_location, token, token_type);
		if (token_type == TOKEN_TYPE::NAME)
		{
			block.name = std::string(token.data);
			//print_tabs(tab_depth);
			//DEBUG_LOG("<" << block.name);

			next_token(buffer, buffersize, current_location, token, token_type);
			while (token_type != TOKEN_TYPE::END_BLOCK)
			{
				if (token_type == TOKEN_TYPE::NAME)
				{
					block.fields.push_back(parse_field(buffer, buffersize, current_location, token, token_type, tab_depth));
				}
				else
				{
					//print_tabs(tab_depth);
					ERROR_LOG("Expecting filed values\n");
				}
				next_token(buffer, buffersize, current_location, token, token_type);
			}

			if (token.length == 2)//'/>' token
			{
				//DEBUG_LOG("/>\n");
				delete[] token.data;
				token.data = nullptr;
				return block;
			}
			else // '>' token
			{
				//DEBUG_LOG(">\n");


				next_token(buffer, buffersize, current_location, token, token_type);
				while (token_type != TOKEN_TYPE::NEW_CLOSE_BLOCK)
				{
					block.values.push_back(parse_block(buffer, buffersize, current_location, token, token_type, tab_depth + 1));

					next_token(buffer, buffersize, current_location, token, token_type);
				}

				next_token(buffer, buffersize, current_location, token, token_type);
				if (token_type == TOKEN_TYPE::NAME)
				{
					next_token(buffer, buffersize, current_location, token, token_type);
					if (token_type == TOKEN_TYPE::END_BLOCK)
					{
						//print_tabs(tab_depth);
						//DEBUG_LOG("</" << block.name << ">\n");
						delete[] token.data;
						token.data = nullptr;
						return block;
					}
				}
			}
		}
	}

	ERROR_LOG("Error parsing block\n");
	delete[] token.data;
	token.data = nullptr;
	return block;
}

FIELD* find_field(BLOCK* block, std::string name)
{

	for (int k = 0; k < block->fields.size(); k++)
	{
		if (block->fields[k].name.compare(name) == 0)
		{
			return &block->fields[k];
		}
	}
}

int find_block(BLOCK* root_block, std::string name, BLOCK** out_block, int startindex = 0)
{
	for (int i = startindex; i < root_block->values.size(); i++)
	{
		if (root_block->values[i].name.compare(name) == 0)
		{
			*out_block = &root_block->values[i];
			return i;
		}
	}
	*out_block = root_block;
	return -1;
}





RawMesh load_dae(std::string filepath)//(char* buffer, int buffersize)
{
	RawMesh raw_mesh = {};

	int buffersize = 0;
	get_filesize(filepath, &buffersize);
	if (buffersize <= 0)
		return raw_mesh;

	buffersize += 1;//Need space for ending character.
	char* buffer = new char[buffersize];
	if (read_buffer(filepath, buffer, (buffersize - 1)) != 0)
		return raw_mesh;

	buffer[buffersize - 1] = '\0';	


	separate_tokens(buffer, buffersize);
	int current_location = -1;
	
	TOKEN token;
	token.data = nullptr;
	TOKEN_TYPE token_type;
	next_token(buffer, buffersize, current_location, token, token_type);
	BLOCK start_block = parse_block(buffer, buffersize, current_location, token, token_type, 0);

	delete[] buffer;


	BLOCK* library_geometries;
	BLOCK* geometry;
	BLOCK* mesh;
	BLOCK* triangles;
	
	find_block(&start_block, "library_geometries", &library_geometries);
	find_block(library_geometries, "geometry",  &geometry);
	find_block(geometry, "mesh", &mesh);
	find_block(mesh, "triangles", &triangles);
	
	BLOCK* positions = 0;
	BLOCK* normals = 0;
	BLOCK* uvs = 0;

	int input_index = 0;
	for(int i = 0; i < 3; i++)
	{
		BLOCK* input_block;
		input_index = find_block(triangles, "input", &input_block, input_index);
		input_index++;

		FIELD* semantic = find_field(input_block, "semantic");
		FIELD* offset = find_field(input_block, "offset");
		int offset_value = std::atoi(offset->value.c_str());

		DEBUG_LOG("OFFSET: " << offset_value);

		BLOCK* source_block = &mesh->values[offset_value];

		if (semantic->value.compare("VERTEX") == 0)
		{
			find_block(source_block, "float_array", &positions);
		}
		else if (semantic->value.compare("NORMAL") == 0)
		{
			find_block(source_block, "float_array", &normals);
		}
		else if (semantic->value.compare("TEXCOORD") == 0)
		{
			find_block(source_block, "float_array", &uvs);
		}
	}

	BLOCK* triangles_data;
	find_block(triangles, "p", &triangles_data);
	FIELD* triangle_count = find_field(triangles, "count");
	int index_counter = -1;
	int num_indices = 3 * std::atoi(triangle_count->value.c_str());

	Vertex* vertex_buffer = new Vertex[num_indices];
	unsigned int* index_buffer = new unsigned int[num_indices];
	for (int i = 0; i < triangles_data->values.size() / 3; i++)
	{
		//Build faces here
		int i0 = i * 3 + 0;
		int i1 = i * 3 + 1;
		int i2 = i * 3 + 2;
		int offset[3] = {(int)triangles_data->values[i0].value, (int)triangles_data->values[i1].value, (int)triangles_data->values[i2].value};
		Vertex v;
		v.position = glm::vec3(
			positions->values[offset[0] * 3 + 0].value, 
			positions->values[offset[0] * 3 + 1].value, 
			positions->values[offset[0] * 3 + 2].value
		);

		v.normal = glm::vec3(
			normals->values[offset[1] * 3 + 0].value,
			normals->values[offset[1] * 3 + 1].value,
			normals->values[offset[1] * 3 + 2].value
		);

		v.uv = glm::vec2(
			uvs->values[offset[2] * 2 + 0].value,
			uvs->values[offset[2] * 2 + 1].value
		);

		float length = glm::length(v.position);
		vertex_buffer[i] = v;
		index_buffer[i] = i;
	}
	
	raw_mesh.index_buffer = index_buffer;
	raw_mesh.vertex_buffer = vertex_buffer;
	raw_mesh.index_count = num_indices;
	raw_mesh.vertex_count = num_indices;
	raw_mesh.mesh_id = 2;

	return raw_mesh;
}