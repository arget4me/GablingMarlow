
#include "dae_loader.h"
#include <Renderer/opengl_renderer.h>
#include <Utils/logfile.h>
#include <Utils/readfile.h>

#include <string>
#include <map>
#include <vector>
#include "globals.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

enum class TOKEN_TYPE {
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

#if 0
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
#endif


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

#if 0
local_scope void print_tabs(int tab_depth)
{
	for (int i = 0; i < tab_depth; i++)
		DEBUG_LOG(" ");
}
#endif

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
	return nullptr;
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

int find_block(BLOCK* root_block, std::string name, BLOCK** out_block, std::string field_name, std::string field_value, int startindex = 0)
{
	for (int i = startindex; i < root_block->values.size(); i++)
	{
		if (root_block->values[i].name.compare(name) == 0)
		{
			FIELD* field = find_field(&root_block->values[i], field_name);
			if(field->value.compare(field_value) == 0)
			{ 
				*out_block = &root_block->values[i];
				return i;
			}
		}
	}
	*out_block = root_block;
	return -1;
}

Frame mat4_to_frame(glm::mat4 matrix)
{
	Frame frame;

	frame.position = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);

	glm::vec3 col_x(matrix[0][0], matrix[0][1], matrix[0][2]);
	glm::vec3 col_y(matrix[1][0], matrix[1][1], matrix[1][2]);
	glm::vec3 col_z(matrix[2][0], matrix[2][1], matrix[2][2]);
	
	frame.size = glm::vec3(glm::length(col_x), glm::length(col_y), glm::length(col_z));

	col_x = glm::normalize(col_x);
	col_y = glm::normalize(col_y);
	col_z = glm::normalize(col_z);
	
	matrix[0] = glm::vec4(col_x, 0);
	matrix[1] = glm::vec4(col_y, 0);
	matrix[2] = glm::vec4(col_z, 0);
	matrix[3] = glm::vec4(0, 0, 0, 1);

	frame.orientation = glm::quat(matrix);

	return frame;
}

int build_node_structure(AnimatedMesh& animation, BLOCK* bone, BLOCK* joint_names)
{
	FIELD* sid = find_field(bone, "sid");
	int bone_index = -1;
	for (int i = 0; i < joint_names->values.size(); i++)
	{
		if (joint_names->values[i].name.compare(sid->value) == 0)
		{
			bone_index = i;
			break;
		}
	}
	Bone& b = animation.bones[bone_index];
	b.num_children = 0;

	int index = 0;

	BLOCK* child;
	while ((index = find_block(bone, "node", &child, "type", "JOINT", index)) != -1)
	{
		int child_index = build_node_structure(animation, child, joint_names);
		
		if (b.num_children < MAX_NUM_CHILDREN)
		{
			b.children[b.num_children++] = (unsigned char)child_index;
		}

		index++;
	}

	return bone_index;
}

#if 0
void print_animation(RawAnimMesh& raw_mesh, AnimatedMesh& animation)
{
	DEBUG_LOG("Num bones: " << (int)animation.num_bones << "\n");
	DEBUG_LOG("Num frames: " << (int)animation.num_frames << "\n\n");

	for (int i = 0; i < (int)animation.num_bones; i++)
	{
		Bone& b = animation.bones[i];
		DEBUG_LOG("Bone" << i << ": num_children=" << (int)b.num_children << " ");
		if((int)b.num_children > 0)
		{
			DEBUG_LOG("children = [")
			for (int k = 0; k < b.num_children; k++)
			{
				DEBUG_LOG((int)b.children[k]);
				if (k != ((int)b.num_children - 1))
				{
					DEBUG_LOG(", ");
				}
				else
				{
					DEBUG_LOG("] ");
				}
			}
		}

		DEBUG_LOG("mat[");
		for (int k = 0; k < 16; k++)
		{
			DEBUG_LOG(b.inv_bind_mat[k%4][k/4]);
			if (k != (16 - 1))
			{
				DEBUG_LOG(" ");
			}
			else
			{
				DEBUG_LOG("]\n");
			}
		}
	}

	DEBUG_LOG("\n");
	for (int i = 0; i < animation.num_frames * animation.num_bones; i++)
	{
		if (i % animation.num_frames == 0)
		{
			DEBUG_LOG("Frames for Bone" << i / animation.num_frames << " ------------------------------------------\n");
		}
		glm::mat4 mat(1.0f);
		DEBUG_LOG("Frame" << i % animation.num_frames <<": Timestamp=" << animation.frames[i].timestamp << " [");
		mat = glm::translate(mat, animation.frames[i].position);
		mat = mat * glm::toMat4(animation.frames[i].orientation);
		for (int k = 0; k < 16; k++)
		{
			DEBUG_LOG(mat[k % 4][k / 4]);
			if (k != (16 - 1))
			{
				DEBUG_LOG(" ");
			}
			else
			{
				DEBUG_LOG("]\n");
			}
		}

	}
	return;


}
#endif

bool load_dae(std::string filepath, RawAnimMesh* out_raw_mesh, AnimatedMesh* out_animated_mesh)
{
	RawAnimMesh raw_mesh = { 0 };
	AnimatedMesh animation = { 0 };

	int buffersize = 0;
	get_filesize(filepath, &buffersize);
	if (buffersize <= 0)
		return false;

	buffersize += 1;//Need space for ending character.
	char* buffer = new char[buffersize];
	if (read_buffer(filepath, buffer, (buffersize - 1)) != 0)
		return false;

	buffer[buffersize - 1] = '\0';


	separate_tokens(buffer, buffersize);
	int current_location = -1;

	TOKEN token;
	token.data = nullptr;
	TOKEN_TYPE token_type;
	next_token(buffer, buffersize, current_location, token, token_type);
	
	BLOCK start_block = parse_block(buffer, buffersize, current_location, token, token_type, 0);
	delete[] buffer;


	//---------------Fill vertex and index buffer-----------------
	BLOCK* library_geometries;
	BLOCK* geometry;
	BLOCK* mesh;
	BLOCK* triangles;

	find_block(&start_block, "library_geometries", &library_geometries);
	find_block(library_geometries, "geometry", &geometry);
	find_block(geometry, "mesh", &mesh);
	find_block(mesh, "triangles", &triangles);

	BLOCK* positions = 0;
	BLOCK* normals = 0;
	BLOCK* uvs = 0;

	int input_index = 0;
	for (int i = 0; i < 3; i++)
	{
		BLOCK* input_block;
		input_index = find_block(triangles, "input", &input_block, input_index);
		input_index++;

		FIELD* semantic = find_field(input_block, "semantic");


		if (semantic->value.compare("VERTEX") == 0)
		{
			BLOCK* source_block = &mesh->values[0];
			find_block(source_block, "float_array", &positions);
		}
		else if (semantic->value.compare("NORMAL") == 0)
		{
			BLOCK* source_block = &mesh->values[1];
			find_block(source_block, "float_array", &normals);
		}
		else if (semantic->value.compare("TEXCOORD") == 0)
		{
			BLOCK* source_block = &mesh->values[2];
			find_block(source_block, "float_array", &uvs);
		}
	}

	BLOCK* triangles_data;
	find_block(triangles, "p", &triangles_data);
	FIELD* triangle_count = find_field(triangles, "count");
	int index_counter = -1;
	int num_indices = 3 * std::atoi(triangle_count->value.c_str());

	AnimVertex* vertex_buffer = new AnimVertex[num_indices];
	unsigned int* index_buffer = new unsigned int[num_indices];



	BLOCK* library_controllers;
	BLOCK* controller;
	BLOCK* skin;
	BLOCK* vertex_weights;

	find_block(&start_block, "library_controllers", &library_controllers);
	find_block(library_controllers, "controller", &controller);
	find_block(controller, "skin", &skin);
	find_block(skin, "vertex_weights", &vertex_weights);

	BLOCK* vertex_weights_counts;
	BLOCK* vertex_weights_values;
	find_block(vertex_weights, "vcount", &vertex_weights_counts);
	find_block(vertex_weights, "v", &vertex_weights_values);


	BLOCK* joint_weights;
	{
		BLOCK* input;
		find_block(vertex_weights, "input", &input, "semantic", "WEIGHT");
		FIELD* source = find_field(input, "source");
		find_block(skin, "source", &joint_weights, "id", source->value.substr(1, source->value.size()));
		find_block(joint_weights, "float_array", &joint_weights);
	}


	int values_index = 0;
	AnimVertex* vertex_array = new AnimVertex[vertex_weights_counts->values.size()];


	//Load weights and positions
	for (int i = 0; i < vertex_weights_counts->values.size(); i++)
	{
		AnimVertex& v = vertex_array[i];
		v.position = glm::vec3(
			positions->values[i * 3 + 0].value,
			positions->values[i * 3 + 1].value,
			positions->values[i * 3 + 2].value
		);
		int num_weights = (int)vertex_weights_counts->values[i].value;



		if (num_weights <= 4)
		{
			for (int k = 0; k < 4; k++)
			{
				if (k < num_weights)
				{
					v.bone_ids[k] = (unsigned int)vertex_weights_values->values[values_index * 2 + 0].value;//Joint ID
					unsigned int weight_index = (unsigned int)vertex_weights_values->values[values_index * 2 + 1].value;
					float joint_weight = joint_weights->values[weight_index].value;

					v.bone_weights[k] = joint_weight;
					values_index++;
				}
				else
				{
					v.bone_ids[k] = 0;
					v.bone_weights[k] = 0.0f;
				}
			}
		}
		else
		{
			v.bone_weights = glm::vec4(-1.0f);
			for (int k = 0; k < num_weights; k++)
			{
				unsigned int joint_id = (unsigned int)vertex_weights_values->values[values_index * 2 + 0].value;//Joint ID
				unsigned int weight_index = (unsigned int)vertex_weights_values->values[values_index * 2 + 1].value;
				float joint_weight = joint_weights->values[weight_index].value;

				int min_index = -1;
				float min_weight = joint_weight;
				for (int w = 0; w < 4; w++)
				{
					if (v.bone_weights[w] < min_weight)
					{
						min_index = w;
						min_weight = v.bone_weights[w];
					}
				}

				if (min_index != -1)
				{
					v.bone_ids[min_index] = joint_id;
					v.bone_weights[min_index] = joint_weight;
				}

				values_index++;
			}

		}

		v.bone_weights = glm::normalize(v.bone_weights);

	}

	//Fill in indices and vertex buffer arrays
	for (int i = 0; i < triangles_data->values.size() / 3; i++)
	{
		//Build Vertex here
		int i0 = i * 3 + 0;
		int i1 = i * 3 + 1;
		int i2 = i * 3 + 2;
		int offset[3] = { (int)triangles_data->values[i0].value, (int)triangles_data->values[i1].value, (int)triangles_data->values[i2].value };
		AnimVertex v = vertex_array[offset[0]];

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

	delete[] vertex_array;//the temporary storage for positions and weights together.

	raw_mesh.index_buffer = index_buffer;
	raw_mesh.vertex_buffer = vertex_buffer;
	raw_mesh.index_count = num_indices;
	raw_mesh.vertex_count = num_indices;
	raw_mesh.mesh_id = 2;
	//------------Vertex and indexbuffer filled--------------------
	


	//----------fill animated mesh-------------
	BLOCK* joint_names;
	BLOCK* inv_bind_mat;
	{
		BLOCK* joints;
		find_block(skin, "joints", &joints);
		{
			BLOCK* input;
			find_block(joints, "input", &input, "semantic", "JOINT");
			FIELD* source = find_field(input, "source");
			find_block(skin, "source", &joint_names, "id", source->value.substr(1, source->value.size()));
			find_block(joint_names, "Name_array", &joint_names);
		}
		{
			BLOCK* input;
			find_block(joints, "input", &input, "semantic", "INV_BIND_MATRIX");
			FIELD* source = find_field(input, "source");
			find_block(skin, "source", &inv_bind_mat, "id", source->value.substr(1, source->value.size()));
			find_block(inv_bind_mat, "float_array", &inv_bind_mat);
		}
	}
	
	animation.num_bones = (unsigned char)joint_names->values.size();

	animation.bones = new Bone[animation.num_bones];
	for (int i = 0; i < animation.num_bones; i++)
	{
		glm::mat4& mat = animation.bones[i].inv_bind_mat;
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				mat[x][y] = inv_bind_mat->values[i * 16 + x + y * 4].value;
			}
		}
	}

	BLOCK* animations;
	find_block(&start_block, "library_animations", &animations);
	find_block(animations, "animation", &animations);

	int anim_bone_index = 0;
	bool set_num_frames = true;
	for (int i = 0; i < animation.num_bones; i++)
	{
		BLOCK* anim_current_bone;
		anim_bone_index = find_block(animations, "animation", &anim_current_bone, anim_bone_index);
		anim_bone_index++;

		BLOCK* sampler;
		int sampler_index = find_block(anim_current_bone, "sampler", &sampler);
		if (sampler_index == -1)
		{
			ERROR_LOG("Bone" << i << " dosesn't have any key frames!\n");
			return false;
		}
		{
			BLOCK* input;
			find_block(sampler, "input", &input, "semantic", "INPUT");
			FIELD* source = find_field(input, "source");
			find_block(anim_current_bone, "source", &input, "id", source->value.substr(1, source->value.size()));
			find_block(input, "float_array", &input);
			if (set_num_frames)
			{
				animation.num_frames = (unsigned char) input->values.size();
				animation.frames = new Frame[animation.num_bones * animation.num_frames];
				set_num_frames = false;
			}

			for (int k = 0; k < animation.num_frames; k++)
			{
				animation.frames[i * animation.num_frames + k].timestamp = input->values[k].value;
			}
		}

		{
			BLOCK* output;
			find_block(sampler, "input", &output, "semantic", "OUTPUT");
			FIELD* source = find_field(output, "source");
			find_block(anim_current_bone, "source", &output, "id", source->value.substr(1, source->value.size()));
			find_block(output, "float_array", &output);
			for (int k = 0; k < animation.num_frames; k++)
			{
				glm::mat4 mat(1.0f);
				for (int y = 0; y < 4; y++)
				{
					for (int x = 0; x < 4; x++)
					{
						mat[x][y] = output->values[k * 16 + x + y * 4].value;
					}
				}

				float timestamp = animation.frames[i * animation.num_frames + k].timestamp;

				animation.frames[i * animation.num_frames + k] = mat4_to_frame(mat);
				animation.frames[i * animation.num_frames + k].timestamp = timestamp;
			}
		}

	}

	BLOCK* library_visual_scenes;
	find_block(&start_block, "library_visual_scenes", &library_visual_scenes);
	find_block(library_visual_scenes, "visual_scene", &library_visual_scenes);
	find_block(library_visual_scenes, "node", &library_visual_scenes);

	BLOCK* bone;
	find_block(library_visual_scenes, "node", &bone, "type", "JOINT");

	build_node_structure(animation, bone, joint_names);

	animation.animation_transforms = new glm::mat4[animation.num_bones];
	
	//----------animated mesh filled-------------

	*out_raw_mesh = raw_mesh;
	*out_animated_mesh = animation;

	//print_animation(raw_mesh, animation);
	
	return true;
}