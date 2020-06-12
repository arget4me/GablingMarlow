#include "obj_loader.h"
#include <Utils/readfile.h>
#include <string>

enum class TOKEN_TYPE{
	VERTEX,
	NORMAL,
	TEXTURE,
	FACE,
	NUM,
	FACEINDEX,
	OTHER
};

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
			loadobj(buffer, filesize);
		}
		
		delete[] buffer;
	}

}
#endif // TEST_LOADOBJ

void separate_tokens(char* buffer, int buffersize)
{
	for (int i = 0; i < buffersize; i++)
	{
		if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r')
		{
			buffer[i] = '\0';
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

bool MatchVertex(char* buffer, int buffersize, int& current_location)
{
	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			char* token = buffer + current_location;
			ERROR_LOG("Can't parse vertex number " << i << ": " << token << "\n");
			return false;
		}
	}

	return true;
}

bool MatchNormal(char* buffer, int buffersize, int& current_location)
{
	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			char* token = buffer + current_location;
			ERROR_LOG("Can't parse vertex normal number " << i << ": " << token << "\n");
			return false;
		}
	}

	return true;
}

bool MatchTexture(char* buffer, int buffersize, int& current_location)
{
	for (int i = 0; i < 2; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		if (!match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			char* token = buffer + current_location;
			ERROR_LOG("Can't parse vertex texture number " << i << ": " << token << "\n");
			return false;
		}
	}

	return true;
}

bool MatchFace(char* buffer, int buffersize, int& current_location)
{
	for (int i = 0; i < 3; i++)
	{
		get_next_token(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		if (match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::NUM))
		{
			int number = std::stoi(token);
			DEBUG_LOG("Face index " << i << " = " << number << "\n");
		}
		else if (match(get_token_type(buffer, buffersize, current_location), TOKEN_TYPE::FACEINDEX))
		{
			DEBUG_LOG("Token = " << token << " |---| ");
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
			int number_1 = -1;
			int number_2 = -1;
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

			DEBUG_LOG("Face Index " << i << " = (" << number_0 << ", " << number_1 << ", " << number_2 << ")\n");
			
			if(slash_index[0] != 0)
				token[slash_index[0]] = '/';

			if (slash_index[1] != 0)
				token[slash_index[1]] = '/';
		}
		else
		{
			ERROR_LOG("Can't parse face index number " << i <<": " << token << "\n");
			return false;
		}
	}

	return true;
}

void loadobj(char* buffer, int buffersize)
{
	separate_tokens(buffer, buffersize);
	int current_location = 0;
	int num_faces = 0; //3 vertices per face
	int num_pos = 0;
	int num_normals = 0;
	int num_textures = 0;
	int num_vertices = 0; //One vertex with pos, normal, uv
	while (current_location < buffersize)
	{
		//Process current token
		int token_size = get_token_size(buffer, buffersize, current_location);
		char* token = buffer + current_location;
		//DEBUG_LOG(token);

		
		TOKEN_TYPE type = get_token_type(buffer, buffersize, current_location);
		if (type == TOKEN_TYPE::VERTEX)
		{
			//DEBUG_LOG(token << " <-- VERTEX\n");
			if (MatchVertex(buffer, buffersize, current_location))
			{
				num_pos++;
			}
		}
		else if (type == TOKEN_TYPE::NORMAL)
		{
			//DEBUG_LOG(token << " <-- NORMAL\n");
			if (MatchNormal(buffer, buffersize, current_location))
			{
				num_normals++;
			}
		}
		else if (type == TOKEN_TYPE::TEXTURE)
		{
			//DEBUG_LOG(token << " <-- TEXTURE\n");
			if (MatchTexture(buffer, buffersize, current_location))
			{
				num_textures++;
			}
		}
		else if (type == TOKEN_TYPE::FACE)
		{
			if (MatchFace(buffer, buffersize, current_location))
			{
				num_faces++;
			}
			//DEBUG_LOG(token << " <-- FACE\n");
		}
		else if (type == TOKEN_TYPE::NUM)
		{
			//DEBUG_LOG(token << " <-- NUM\n");
		}
		else if (type == TOKEN_TYPE::FACEINDEX)
		{
			//DEBUG_LOG(token << " <-- FACEINDEX\n");
		}
		else if (type == TOKEN_TYPE::OTHER)
		{
			//DEBUG_LOG(token << " <-- OTHER\n");
		}
		


		get_next_token(buffer, buffersize, current_location);

	}
	DEBUG_LOG("Num faces: " << num_faces << "\n");
	DEBUG_LOG("Num pos: " << num_pos << "\n");
	DEBUG_LOG("Num normals: " << num_normals << "\n");
	DEBUG_LOG("Num textures: " << num_textures << "\n");
}